/*

 ClipSeekr 1.0
 Copyright (C) 2011 by Aetilius, Inc.
 All rights reserved.

 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <dirent.h>
#include <thread>

#include <VideoCapture.hpp>
#include <pHashPro.h>
#include "TableEntry.hpp"
#include "RedisHashStore.hpp"
#include "clipseekrutils.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace po = boost::program_options;
using namespace boost::filesystem;


struct Args {
	path dir_name;  /* directory of files to add */
	string host;      /* redis server address */
	string id;        /* a target id to delete */
	int port;         /* redis port */
	bool overwrite;   /* overwrite entries */
	bool list;        /* list entries in db */
	float rho;        /** gaussian smoothing parameter */
	int tm;
	int bm;           /* top, bottom, left, right margin args */
	int lm;
	int rm;
	int dct_blk_offset; /* offset of dct block on frame*/
};
/**
 *  Return files in given directory
 * 
 **/
vector<string> ReadFileNames(const path &dirname) {
  directory_iterator dir(dirname), end;
  vector <string> files;
  for (; dir != end; ++dir) {
    if (is_regular_file(dir->status()))
      files.push_back(dir->path().string());
  }
  return files;
}

void print_header(){
  cout << "*******************************" << endl;
  cout << "Clipster " << CLIPSEEKR_VERSION << ". Aetilius, Inc. 2014" << endl;
#ifdef CLIPSEEKR_TRIAL_VERSION
  cout << "Trial Version. " << endl;
#endif
  cout << "*******************************" << endl << endl;
}

Args ParseOptions(int argc, char **argv){
	Args args;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("dir,d",po::value<path>(&args.dir_name), "directory of files to hash")
		("server,s",po::value<string>(&args.host)->default_value("tcp://localhost"),
		 "redis server hostname or unix domain socket path (tcp://<server> or unix://<path>)")
		("port,p", po::value<int>(&args.port)->default_value(6379),"redis server port")
		(",o",po::value<bool>(&args.overwrite)->default_value(false),"track & print overwrites")
		("delete",po::value < string > (&args.id), "id of file to delete,")
		("list", po::value<bool>(&args.list)->default_value(false), "list entries in database")
		("rho,r", po::value<float>(&args.rho)->default_value(1.0), "gaussian smoothing parameter for each frame")
		("tm" , po::value<int>(&args.tm)->default_value(0), "top margin crop")
		("bm" , po::value<int>(&args.bm)->default_value(0), "bottom margin crop")
		("lm" , po::value<int>(&args.lm)->default_value(0), "left margin crop")
		("rm" , po::value<int>(&args.rm)->default_value(0), "right margin crop")
		("block", po::value<int>(&args.dct_blk_offset)->default_value(1), "offset of dct hash block [1,24] e.g. 1,4");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
 
  if (vm.count("help")) {
    cout << desc << endl;
    exit(0);
  }

  if(vm.count("delete") && vm.count("list")) {
    cerr << "Only one option allowed: list or delete" << endl;
	exit(0);
  }

  return args;
}

void process_main(ph::VideoCapture *vc){
	assert(vc != NULL);
	try {
		vc->Process();
	} catch (ph::VideoCaptureException &ex){
		cerr << "unable to process packets: " << ex.what() << endl;
	}
}

void process_video(ph::VideoCapture *vc, string id, int idnum, int total, HashStore *store,
				   bool overwrite, const float rho, int blk_idx){
	assert(vc != NULL);
	assert(store != NULL);
	try {
		ClipTBLEntry entry;
		entry.id = id;
		entry.idnum = idnum;
		entry.total = total;
		int count = 0;
		AVFrame *pframe = NULL;
		while ((pframe = vc->PullVideoFrame()) != NULL){
			entry.seqnum = count++;
			struct ImgHash hash;
			hash.hashc = 0;
			if (framehash(pframe->data[0], pframe->width, pframe->height,
									 pframe->linesize[0], rho, &hash.hashc, blk_idx) < 0){
				throw runtime_error("unable to calculate hash value");
			}
			store->PutHashKeyValue(&hash, &entry, overwrite);
			av_frame_free(&pframe);
		}
		cout << " added " << count << " frames." << endl;
	} catch (ph::VideoCaptureException &ex){
		cerr << "unable to pull frames: " << ex.what() << endl;
	} catch (RedisStoreException &ex){
		cerr << "unable to store hash key/value pair: " << ex.what() << endl;
	} catch (runtime_error &ex){
		cerr << "unable to hash frames" << ex.what() << endl;
	}
}

void add_file(const string file, int idnum, HashStore *store, const Args &args){
	assert(store != NULL);
	int flag = PHCAPTURE_VIDEO_FLAG;
	try {
		string idstr = string(strrchr(file.c_str(), '/') + 1);
		ph::VideoCapture *vc = new ph::VideoCapture(file, args.tm, args.bm, args.lm, args.rm, 0, -1, flag, 0);
		assert(vc != NULL);
		uint32_t nbframes = vc->CountVideoPackets();
		thread process_thr = thread(process_main, vc);
		thread video_thr = thread(process_video, vc, idstr, idnum, nbframes, store, args.overwrite,
								  args.rho, args.dct_blk_offset);
		process_thr.join();
	    video_thr.join();
		delete vc;
	} catch (ph::VideoCaptureException &ex){
		cerr << "unable to open file: " << ex.what() << endl;
	} catch (RedisStoreException &ex){
		cerr << "unable to insert key/value to db: " << ex.what() << endl;
	}
}

int main(int argc, char **argv) {
  print_header();
  
  Args args = ParseOptions(argc, argv);

  uint32_t nbentries = 0;
  RedisHashStore *store = NULL;
  int count;
  try {
	  store = new RedisHashStore(args.host, args.port);
	  if (store == NULL){
		  cerr << "unable to open redis store" << endl;
		  exit(EXIT_FAILURE);
	  }
	  count = store->GetCount();
	  nbentries = count;
	  cout << count << " hash entries already in db." << endl;
  } catch (RedisStoreException &ex){
	  cerr << "unable to open redis db connection: " << ex.what() << endl;
	  return 0;
  }

  if(args.list) {
	  cout << "List entries in db." << endl;
	  vector<string> ids = store->GetEntries();
	  for(std::vector<string>::iterator it = ids.begin(); it != ids.end(); ++it) {
		  cout << *it << endl;
	  }
	  return 0;
  }

  if (args.dir_name.empty()){
	  cerr << "no directory arg given" << endl;
	  exit(EXIT_FAILURE);
  }
  vector<string> files = ReadFileNames(args.dir_name.string());
  if (files.empty()){
	  cerr << "no files in directory: " << args.dir_name << endl;
	  exit(EXIT_FAILURE);
  }
  
  unsigned int nbfiles = files.size();
  if (nbfiles < 1) {
    cout << "unable to read files" << endl;
	goto cleanup;
  }
  
#ifdef CLIPSEEKR_TRIAL_VERSION
  nbfiles = (nbfiles <= 10) ? nbfiles : 10;
#endif
  cout << "Processing " << nbfiles << " files in " << args.dir_name << " ... " << endl;
 
  for (int i =0;i<(int)nbfiles;i++) {
	  string name = string(strrchr(files[i].c_str(), '/') + 1);
	  cout << "file[" << i << "] " << name << "  ";
	  add_file(files[i], ++nbentries, store, args);
  }
 
cleanup:
  cout << "Done." << endl;
  delete store;
  return 0;
}
