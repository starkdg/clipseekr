#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <vector>
#include "RedisHashStore.hpp"
#include "TableEntry.hpp"

using namespace std;

static string m_idname = "idname";

uint64_t assign_random_long(){
  uint64_t result = rand();
  result <<= 32;
  result |= rand();
  return result;
}

void assert_equal_table_entrys(const ClipTBLEntry &entry1, const ClipTBLEntry &entry2){
  assert(entry1.id     == entry2.id);
  assert(entry1.idnum  == entry2.idnum);
  assert(entry1.seqnum == entry2.seqnum);
  assert(entry1.total  == entry2.total);
}

static int m_idvalue = 0;

int create_keyvalue_set(vector<struct ImgHash> &hashes, vector<ClipTBLEntry> &entries, const int N){
  int current_id = ++m_idvalue;
  hashes.clear();
  entries.clear();
  hashes.reserve(N);
  entries.reserve(N);

  struct ImgHash hash;
  ClipTBLEntry entry;

  entry.id = m_idname + to_string(current_id);
  entry.idnum = current_id;
  entry.total = N;
  for (int i=0;i<N;i++){
    hash.hashc = assign_random_long();    
    entry.seqnum = i;
    hashes.push_back(hash);
    entries.push_back(entry);
  }
  return 0;
}

void prime_random_generator(){
  const unsigned int seed = 29839813U;
  srand(seed);
  for (int i=0;i<2000;i++){
    rand();
  }
}

void run_test(const string &host, const int port){
  int N1 = 55, N2 = 125, N3 = 980, N4 = 4500;

  vector<struct ImgHash> keys1;
  vector<ClipTBLEntry> entries1;
  create_keyvalue_set(keys1, entries1, N1);

  vector<struct ImgHash> keys2;
  vector<ClipTBLEntry> entries2;
  create_keyvalue_set(keys2, entries2, N2);

  vector<struct ImgHash> keys3;
  vector<ClipTBLEntry> entries3;
  create_keyvalue_set(keys3, entries3, N3);

  vector<struct ImgHash> keys4;
  vector<ClipTBLEntry> entries4;
  create_keyvalue_set(keys4, entries4, N4);
  
  ClipTBLEntry *plookup = new ClipTBLEntry();
  
  try {
    HashStore *pStore = new RedisHashStore(host, port); //open data store
    assert(pStore != NULL);

    int initial_no_entries = pStore->GetCount();
    cout << "initial no. entries " << initial_no_entries << endl;
    assert(initial_no_entries >= 0);

    vector<string> ids = pStore->GetEntries();
    int initial_no_ids = ids.size();
    cout << "initial no. id's " << initial_no_ids << endl;
    assert(initial_no_ids >= 0);

    cout << "Add entries." << endl;
    // add first group of key/value pairs
    for (int i=0;i<N1;i++){
      pStore->PutHashKeyValue(&keys1[i], &entries1[i], true);
    }
	   
    // add second group of key/value pairs
    for (int i=0;i<N2;i++){
      pStore->PutHashKeyValue(&keys2[i], &entries2[i], true);
    }

    // add third group of key value pairs
    for (int i=0;i<N3;i++){
      pStore->PutHashKeyValue(&keys3[i], &entries3[i], true);
    }

    // add fourth group of key value pairs
    for (int i=0;i<N4;i++){
      pStore->PutHashKeyValue(&keys4[i], &entries4[i], true);
    }


    cout << "Test Retrieval." << endl;
    int index = rand()%N1;
    pStore->GetHashValue(&keys1[index], &plookup);
    assert_equal_table_entrys(entries1[index], *plookup);

    index = rand()%N2;
    pStore->GetHashValue(&keys2[index], &plookup);
    assert_equal_table_entrys(entries2[index], *plookup);


    index = rand()%N3;
    pStore->GetHashValue(&keys3[index], &plookup);
    assert_equal_table_entrys(entries3[index], *plookup);

    index = rand()%N4;
    pStore->GetHashValue(&keys4[index], &plookup);
    assert_equal_table_entrys(entries4[index], *plookup);

    
    int n = pStore->GetCount();
    assert(n == N1 + N2 + N3 + N4 + initial_no_entries);

    n = pStore->GetCount(entries1[0].id);
    assert(n == N1);

    n = pStore->GetCount(entries2[0].id);
    assert(n == N2);

    n = pStore->GetCount(entries3[0].id);
    assert(n == N3);

    n = pStore->GetCount(entries4[0].id);
    assert(n == N4);

    vector<string> ids2 = pStore->GetEntries();
    assert(ids2.size() == 4);
    

    cout << "Delete entries." << endl;
    for (int i=0;i<N1;i++){
      pStore->DeleteEntry(&keys1[i]);
    }

    for (int i=0;i<N2;i++){
      pStore->DeleteEntry(&keys2[i]);
    }

    for (int i=0;i<N3;i++){
      pStore->DeleteEntry(&keys3[i]);
    }

    for (int i=0;i<N4;i++){
      pStore->DeleteEntry(&keys4[i]);
    }
    
    n = pStore->GetCount();
    assert(n == initial_no_entries);

    n = pStore->GetCount(entries1[0].id);
    assert(n == 0);

    n = pStore->GetCount(entries2[0].id);
    assert(n == 0);

    n = pStore->GetCount(entries3[0].id);
    assert(n == 0);

    n = pStore->GetCount(entries4[0].id);
    assert(n == 0);

    vector<string> ids3 = pStore->GetEntries();
    assert(ids3.size() == 0);

    
    cout << "Add ID." << endl;
    for (int i=0;i<N1/2;i++){
      pStore->PutHashKeyValue(&keys1[i], &entries1[i], true);
    }

    cout << "Delete by ID" << endl;
    pStore->DeleteEntry(entries1[0].id);
    
    cout << "Test Overwrite." << endl;
    pStore->PutHashKeyValue(&keys1[10], &entries1[10], false); // put in one entry for given key
    pStore->PutHashKeyValue(&keys1[10], &entries1[12], true);  // overwrite another entry to same key
    pStore->GetHashValue(&keys1[10], &plookup);
    assert_equal_table_entrys(entries1[12], *plookup);

    pStore->DeleteEntry(&keys1[10]);
   
    vector<string> ids4 = pStore->GetEntries();
    cout << "final no. id's " << ids4.size() << endl;
    assert((int)ids4.size() == initial_no_ids);

    n = pStore->GetCount();
    cout << "final no. entries " << n << endl;
    assert(n == initial_no_entries);
    
    
    delete pStore;
  } catch (RedisStoreException &ex){
    cout << "unable to complete redis op: " << ex.what() << endl;
    assert(false);
  }

  delete plookup;
  return;
}

int main(int argc, char **argv){
  const string host = "tcp://127.0.0.1";
  const int port = 6379;

  prime_random_generator();

  cout << "Test Redis Key/Value Hash Store." << endl;
  run_test(host, port);
  
  cout << "Done." << endl;
  return 0;
}
