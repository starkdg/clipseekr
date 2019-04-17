#ifndef REDISHASHSTORE_H
#define REDISHASHSTORE_H

#include <string>
#include <stdexcept>
#include <exception>
#include <sys/socket.h>
#include <netinet/in.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "HashStore.hpp"
#include "hiredis.h"

class RedisHashStore: public HashStore {
private:
  redisContext *ctx;
  int port;
  string host;
  void check_redis_reply(int r, redisReply *reply);

public:
  void printEntry(const struct ImgHash *pimghash, ClipTBLEntry *e);
  ~RedisHashStore();
  RedisHashStore();
  RedisHashStore(const string &host, const int port = 6379);
  void PutHashKeyValue(const struct ImgHash *pimghash, const ClipTBLEntry *pentry,bool overwrite);
  void GetHashValue(const struct ImgHash *pimghash, ClipTBLEntry **pentry);
  void DeleteEntry(const struct ImgHash *pimghash);
  void DeleteEntry(const string &id);
  int64_t GetCount();
  int64_t GetCount(const string &id);
  vector<string> GetEntries();
  void Shutdown();
};


class RedisStoreException : public runtime_error {
public:
    RedisStoreException(const string &str):runtime_error(str){};
};

#endif // REDISHASHSTORE_H


