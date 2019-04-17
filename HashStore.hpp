#ifndef HASHSTORE_H
#define HASHSTORE_H

#include <cstdint>
#include <vector>
#include "TableEntry.hpp"

using namespace std;

class HashStore {

public:
  virtual void printEntry(const struct ImgHash *pimghash, ClipTBLEntry *e) = 0;
  HashStore(){};
  virtual ~HashStore(){};
  virtual void PutHashKeyValue(const struct ImgHash *pimghash, const ClipTBLEntry *pentry, bool overwrite)=0;
  virtual void GetHashValue(const struct ImgHash *pimghash, ClipTBLEntry **pentry)=0; 
  virtual void DeleteEntry(const struct ImgHash *pimghash)=0;
  virtual void DeleteEntry(const string &id)=0;
  virtual int64_t GetCount() = 0;
  virtual int64_t GetCount(const string &id)=0;
  virtual vector<string> GetEntries()=0;
  virtual void Shutdown()=0;
};

#endif // HASHSTORE_H
