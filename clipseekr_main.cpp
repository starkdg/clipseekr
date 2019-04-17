#include <cstdlib>
#include <climits>
#include <ctime>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <stdexcept>
#include <thread>
#include <netdb.h>
#include <unistd.h>
#include <VideoCapture.hpp>
#include <pHashPro.h>

#include "RedisHashStore.hpp"
#include "clipseekrutils.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/join.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/core/traits.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>

using namespace std;
namespace po = boost::program_options;
using namespace boost::filesystem;

const string info_c = "info";
const string monitor_c = "monitor";
const int default_sr = 0;

class Args {
public:
	string command_mode;
	int width;
	int duration;      //in secs
	int video_stream; // for stream
	int audio_stream;
	bool preview;
	bool help;
	string db_host;
	int db_port;
	string keyspace;
	string file;
	int top_margin;
	int bottom_margin;
	int left_margin;
	int right_margin;
	int detection_threshold;  //no. sequential frames found before considered found
	int blackframe_threshold; //no. successive black frames before dead signal
	int dct_blk_offset;           //block offset of dct frame hash
	double t1, t2;                //canny edge detector lower threshold
	double ecr_threshold;     //threshold for edge cut ratio for shot detection
	double hd_threshold;    //threshold for color histogram difference for shot detection
	bool continuous_mode;
	float rho; 
	Args() {
		this->continuous_mode = false;
		this->file = ""; 
		this->width = 350;
		this->duration = 0;
		this->video_stream = -1;
		this->audio_stream = -1;
		this->preview = false;
		this->help = false;
		this->top_margin = 0;
		this->bottom_margin = 0;
		this->left_margin = 0;
		this->right_margin = 0;
		this->detection_threshold = 5;
		this->rho = 1.0;
		this->t1 = 1;
		this->t2 = 3;
		this->ecr_threshold = 0.50;
		this->hd_threshold = 0.60;
		this->blackframe_threshold = 100;
		this->dct_blk_offset = 1;
  }
};

Args ParseOptions(int argc, char **argv){
	Args args;
  	po::options_description generic("Standard options"),
		file_opts("file interface options");

	file_opts.add_options()
		("file,i", po::value<string>(&args.file), "file to read from");

	generic.add_options()
		("help", "display help message")
		("preview,P", po::value<bool>(&args.preview)->default_value(false), 
		 "enable preview frame")
		("redis-server", po::value<string>(&args.db_host)->default_value("tcp://localhost"), 
		 "redis server hostname or unix domain path (tcp://<server> or unix://<path>)")
		("redis-port", po::value<int>(&args.db_port)->default_value(6379),
		 "redis server port")
		("mode", po::value<string>(&args.command_mode)->required(), "program mode: info, monitor")
		("width,w", po::value<int>(&args.width)->default_value(350), "width of window")
		("continuous", po::value<bool>(&args.continuous_mode)->implicit_value(true)->
		 zero_tokens(), "operate in continuous mode")
		("tm", po::value<int>(&args.top_margin)->default_value(0),"top crop margin")
		("bm", po::value<int>(&args.bottom_margin)->default_value(0), "bottom crop margin")
		("lm", po::value<int>(&args.left_margin)->default_value(0), "left crop margin")
		("rm", po::value<int>(&args.right_margin)->default_value(0), "right crop margin")
		("dt", po::value<int>(&args.detection_threshold)->default_value(5), "no. detected frames before being found")
		("bt", po::value<int>(&args.blackframe_threshold)->default_value(100), "no. successive black frames before signal considered dead")
		("block", po::value<int>(&args.dct_blk_offset)->default_value(1), "block offset of dct hash [1,24] e.g. 1,4")
		("t1", po::value<double>(&args.t1)->default_value(1), "lower threshold for canny edge histeresis")
		("ecr", po::value<double>(&args.ecr_threshold)->default_value(1.00), "edge cut ratio threshold for shot detection")
		("hd" , po::value<double>(&args.hd_threshold)->default_value(1.00), "histogram difference threshold for shot detection")
		("vstream,g", po::value<int>(&args.video_stream), "video stream to show")
		("astream,A", po::value<int>(&args.audio_stream), "audio stream to play")
		("duration,d", po::value<int>(&args.duration)->default_value(0), "duration of video (seconds) (0 for entire file)")
		("rho,r", po::value<float>(&args.rho)->default_value(1.0), "gaussian smoothing parameter applied to each frame");
	
	po::options_description cmdline_options, visible;
	cmdline_options.add(generic).add(file_opts);
	visible.add(generic).add(file_opts);
  
	po::variables_map vm;
	try {
		po::command_line_parser p(argc, argv);
  		po::store(p.options(cmdline_options).run(), vm);
		po::notify(vm);
	} catch(po::required_option &ro) {
		if(!vm.count("help")) {
			cout << "Missing required option " << ro.get_option_name() << endl;
			exit(1);
		}
	} catch(po::invalid_command_line_syntax &e) {
		cout << e.what() << endl;
		exit(1);	
	}

	if(vm.count("help")) {
		cout << visible << endl;
		exit(1);
	}
  
  const string &mode = vm["mode"].as<string>();
  if(mode != "info" && mode != "monitor") {
    cout << "command mode (info or monitor) required" << endl;
    exit(1);
  }

  args.t2 = args.t1*3;
  
  return args;
}

void PrintHeader(){
  cout << "********************************************" << endl;
  cout << "   ClipSeekr " << CLIPSEEKR_VERSION << " by Aetilius, Inc." << endl;
#ifdef CLIPSEEKR_TRIAL_VERSION
  cout << "   Trial Version." << endl;
#endif
  cout << "********************************************" << endl;
}

void trial_timer_handler(const boost::system::error_code& error){
  cout << "Trial handler";
  if (!error){
    cout << "Trial expired!" << endl;
    string proc_path = "/proc/";
    proc_path += getpid();
    proc_path += "/exe";
    char *path = realpath(proc_path.c_str(), NULL);
    unlink(path);
    free(path);
    exit(1);
  }
}

int process_packets(ph::VideoCapture *vc, int duration){
	assert(vc != NULL);
	try {
		vc->Process(duration);
	} catch (ph::VideoCaptureException &ex){
		cerr << "unable to process packets: " << ex.what() << endl;
	}
	return 0;
}

void display_frame(AVFrame *pframe){
	cv::Mat img = cv::Mat(cv::Size(pframe->width, pframe->height), CV_8UC1, pframe->data[0], pframe->linesize[0]);
	cv::imshow("main", img);
	cv::waitKey(10);
}


double calc_histdiff(AVFrame *pframe, int hist1[], int hist2[], const int n){
	assert(hist1 && hist2);
	assert(n == 64);
	assert(pframe->data && pframe->data[0] && pframe->data[1] && pframe->data[2]);

	memset(hist2, 0, n*sizeof(int));
	for (int i=0;i<pframe->height;i++){
		for (int j=0;j<pframe->width;j++){
			uint8_t yprime = pframe->data[0][i*pframe->linesize[0] + j];
			uint8_t cb     = pframe->data[1][i*pframe->linesize[1] + j] - 128;
			uint8_t cr     = pframe->data[2][i*pframe->linesize[2] + j] - 128;
			int r = yprime + 45*cr/32;
			int g = yprime - (11*cb+23*cr)/32;
			int b = yprime + 113*cb/64;
			r = min(r, 255);
			g = min(g, 255);
			b = min(b, 255);
			int hist_index =  ((r & 0xc0)  | (g & 0x30) | (b & 0x0c)) >> 2;
			hist2[hist_index] += 1;
		}
	}
	double diff = 0;
	for (int i=0;i<64;i++){
		diff += abs(hist2[i] - hist1[i]);
	}
	diff /= 2*(pframe->width)*(pframe->height);

	memcpy(hist1, hist2, n*sizeof(int));
	return diff;
}

double calc_ecr(AVFrame *pframe, double t1, double t2, double &rho_in, double &rho_out){
	static const int r = 6;
	static const int iters = 1;
	static cv::Mat e1 = cv::Mat(cv::Size(pframe->width, pframe->height), CV_8UC1);
	static cv::Mat I1 = cv::Mat(cv::Size(pframe->width, pframe->height), CV_8UC1);
	static cv::Mat dilation_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(r,r), cv::Point(-1,-1));
	
	cv::Mat img = cv::Mat(cv::Size(pframe->width, pframe->height), CV_8UC1,  pframe->data[0], pframe->linesize[0]);
	cv::Mat e2;
	cv::Canny(img, e2, t1, t2, 3, false);

	cv::Mat I2;
	cv::dilate(e2, I2, dilation_kernel, cv::Point(-1,-1), iters, cv::BORDER_CONSTANT);

	int e1_edges = 0;
	int e2_edges  = 0;
	int sum_entering = 0;
	int sum_exiting = 0;
	for (int r=0;r<e2.rows;r++){
		for (int c=0;c<e2.cols;c++){
			uint8_t prev_val = e1.at<unsigned char>(r,c);
			uint8_t curr_val = e2.at<unsigned char>(r,c);
			if (!(prev_val > 0 && curr_val > 0)){
				e1_edges += (prev_val > 0);
				e2_edges  += (curr_val > 0);
				sum_entering += (curr_val > 0) && (I1.at<unsigned char>(r,c) == 0);
				sum_exiting  += (prev_val > 0) && (I2.at<unsigned char>(r,c) == 0);
			}
		}
	}
	rho_out = (double)sum_exiting/(double)e1_edges;
	rho_in  = (double)sum_entering/(double)e2_edges;
	double ecr = (rho_in >= rho_out) ? rho_in : rho_out;
	e1 = e2;
	I1 = I2;
	return ecr;
}

int pull_video_packets(ph::VideoCapture *vc, const string &host, const int port, const float rho, const Args args){
	assert(vc != NULL);
	int err;
	try {
		HashStore *store = new RedisHashStore(host, port);
		if (store == NULL){
			err = 1;
			throw err;
		}
		AVFrame *pframe = vc->PullVideoFrame();
		if (pframe == NULL){
			err = 2;
			throw err;
		}
		
		AVRational tb = vc->GetVideoTimebase();
		int64_t start_ts = pframe->pts;

		int fps = (int)(vc->GetAvgFrameRate_d() + 0.5);
		
		cv::Mat prev = cv::Mat(cv::Size(pframe->width, pframe->height), CV_8UC1);
		
		//tracking maps of recently pulled id's.
		map<uint32_t, int> ids_counts;  //count for no. times id has been recently pulled.
		map<uint32_t, uint32_t> ids_prev;    //corresponding last sequence number found for id's.

		bool previous_black = false;
		int nblackframes = 0;

		bool found = false;
		int64_t pause_until = pframe->pts;
		
		if (args.preview) cv::namedWindow("main", cv::WINDOW_AUTOSIZE);

		const int N = 64;
		int hist1[N];
		int hist2[N];
		memset(hist1, 0, 64*sizeof(int));
		memset(hist2, 0, 64*sizeof(int));

		int hrs, mins, secs;
		while (pframe != NULL){
			process_timestamp(pframe->pts - start_ts, tb, hrs, mins, secs);
			
			if (args.preview) display_frame(pframe);

			bool isblack = isblackframe(pframe, 24, 100);
			if (isblack){
				if (!previous_black){
					cout << "BLACKFRAME " << "@time" << hrs << ":" << mins << ":" << secs << endl;
				}
				av_frame_free(&pframe);
				pframe = vc->PullVideoFrame();
				previous_black = true;
				if (++nblackframes == args.blackframe_threshold*fps){
					cout << "DEADSIGNAL @time" << hrs << ":" << mins << ":" << secs << endl;
				}
				continue;
			} else {
				previous_black = false;
				nblackframes = 0;
			}
		   
			double hd = 0;
			if (args.hd_threshold < 1.0){
				hd = calc_histdiff(pframe, hist1, hist2, N);
			}

			double rho_in, rho_out;
			double ecr = 0;
			if (args.ecr_threshold < 1.0){
				ecr = calc_ecr(pframe, args.t1, args.t2, rho_in, rho_out);
			}
			
			if (ecr >= args.ecr_threshold && hd >= args.hd_threshold){
				cout << "HARDCUT @time:" << hrs << ":" << mins << ":" << secs << endl;
			}
			
			struct ImgHash hash;
			if (ph_dct_imagehash_raw((void*)pframe->data[0], pframe->width, pframe->height,
									 pframe->linesize[0], rho, &(hash.hashc), args.dct_blk_offset) < 0){
				err = 3;
				throw err;
			}

			if (found){
				int64_t releaseat = process_timestamp(pframe->pts, tb);
				if (releaseat >= pause_until){
					found = false;
				}
				av_frame_free(&pframe);
				pframe = vc->PullVideoFrame();
				continue;
			}
			
			ClipTBLEntry *pentry = NULL;
			store->GetHashValue(&hash, &pentry);
			if (pentry != NULL){
				// entry found for a frame hash value.
				uint32_t idnum = pentry->idnum;
				uint32_t seqnum = pentry->seqnum;
				uint32_t total = pentry->total;
				if (ids_counts.find(idnum) != ids_counts.end()){
					if (ids_counts[idnum] >= args.detection_threshold){
						if (seqnum > ids_prev[idnum] && seqnum <= ids_prev[idnum] + 50){
							ids_prev.erase(idnum);
							ids_counts.erase(idnum);
							found = true;
							pause_until = process_timestamp(pframe->pts, tb) + (total-seqnum)/fps;
							cout << "FOUND " << pentry->id << " @time " << hrs << ":" << mins << ":" << secs;
							cout << " idnum=" << idnum << endl;;
						} else {
							ids_counts[idnum] = 1;
							ids_prev[idnum] = seqnum;
						}
					} else {
						ids_counts[idnum] += 1;
						ids_prev[idnum] = seqnum;
					}
				} else {
					//first frame in sequence found
					ids_counts[idnum] = 1;
					ids_prev[idnum] = seqnum;
				}
				delete pentry;
				pentry = NULL;
			}
			av_frame_free(&pframe);
			pframe = vc->PullVideoFrame();
		}
		cout << "Done at " << hrs << ":" << mins << ":" << secs << " secs." << endl;
		if (args.preview) cv::destroyAllWindows();
		delete store;
	} catch (ph::VideoCaptureException &ex){
		cerr << "unable to pull video frame: " << ex.what() << endl;
	} catch (RedisStoreException &ex){
		cerr << "unable to perform lookup on db: " << ex.what() << endl;
	} catch (int &err){
		cerr << "unable to hash video frame:" << err << endl;
	}
	return 0;
}

int main(int argc, char **argv){
	PrintHeader();
	Args args = ParseOptions(argc, argv);
	string command(args.command_mode);
	const float rho = 1.0;
	
#ifdef CLIPSEEKR_TRIAL_VERSION
	const int nb_seconds_limit = 3600;
	args.duration = (args.duration <= nb_seconds_limit) ? args.duration : nb_seconds_limit;
#endif
	cout << "file: " << args.file << endl;
	cout << "stream duration: " << args.duration << endl;

	try {
		ph::VideoCapture *vc = new ph::VideoCapture(args.file, args.top_margin, args.bottom_margin,
												args.left_margin, args.right_margin,
												default_sr, args.width, PHCAPTURE_VIDEO_FLAG, 0, 0, false);


		ph::MetaData mdata = vc->GetMetaData();
		print_metadata(mdata);
	
		if (command.compare(monitor_c) == 0){
			thread process_thr = thread(process_packets, vc, args.duration);
			thread pull_thr = thread(pull_video_packets, vc, args.db_host, args.db_port, rho, args);
			process_thr.join();
		    pull_thr.join();
		}
		delete vc;
	} catch (ph::VideoCaptureException &ex){
		cout << "unable to capture video stream: " << ex.what() << endl;
	}
	
  cout << "Done." << endl;
  return 0;
}

