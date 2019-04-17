#ifndef TABLEHASHSTORE_H
#define TABLEHASHSTORE_H

#include <string>
#include <stdexcept>
#include "HashStore.hpp"

using namespace std;

class TableHashStore : public HashStore {
protected:

public:
    TableHashStore(const string &tablefile, const string &tmptablefile);
    ~TableHashStore();
    void PutHashKeyValue(const struct ImgHash *pimghash, ClipTBLEntry *pentry, bool overwrite);
    void GetHashValue(const struct ImgHash *pimghash, ClipTBLEntry **pentry);
    void PutTmpHashKeyValue(const struct ImgHash *pimghash, TmpClipTBLEntry *pentry, bool overwrite);
    void GetTmpHashValue(const struct ImgHash *pimghash, TmpClipTBLEntry **pentry);
    void DeleteTmpHashValue(const struct ImgHash *pimghash);
    int64_t GetCount();
    void Shutdown();
};

class TableException : public runtime_error {
public:
    TableException(const string &str):runtime_error(str){};
};

#endif // TABLEHASHSTORE_H
