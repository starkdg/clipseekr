#include "RedisHashStore.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

static const vector<string> column_names = { "ID", "IDNUM", "SEQNUM", "TOTAL" };

#define CHECK(X) if ( !X || X->type == REDIS_REPLY_ERROR || ctx->err ) { printf("Error\n"); exit(-1); }


void RedisHashStore::check_redis_reply(int r, redisReply *reply) {
  if (reply == NULL || ctx->err || reply->type == REDIS_REPLY_ERROR || r == REDIS_ERR) {
    throw RedisStoreException(string("Redis error: ") + ctx->errstr);
  }
}

void RedisHashStore::printEntry(const struct ImgHash *pimghash, ClipTBLEntry *e) {
  fprintf(stdout, "%s\t%" PRIu64 "\t%" PRIu32 "\t%" PRIu32 "\t%" PRIu32 "\n",
	  e->id.c_str(), pimghash->hashc,e->idnum, e->seqnum, e->total);
}


RedisHashStore::RedisHashStore(){};

RedisHashStore::RedisHashStore(const string &host, const int port) {
  this->host = host;
  this->port = port;
  if (boost::starts_with(host, "unix://")) {
    ctx = redisConnectUnix(host.substr(7).c_str());
  } else if (boost::starts_with(host, "tcp://")) {
    ctx = redisConnect(host.substr(6).c_str(), port);
  }  else {
    throw runtime_error("Invalid host specifier: " + host);
  }
  if (ctx == NULL || ctx->err) {
    throw RedisStoreException(string("Can't connect to redis at " + host + " err: " + ctx->errstr));
  }
}


RedisHashStore::~RedisHashStore() {
  Shutdown();
}


void RedisHashStore::PutHashKeyValue(const struct ImgHash *pimghash,const ClipTBLEntry *pentry,
				                                     bool overwrite){
  redisReply *reply = NULL;
  redisAppendCommand(ctx, "SELECT 0");
  int r = redisGetReply(ctx, (void **) &reply );
  check_redis_reply(r, reply);
  freeReplyObject(reply);

  if (!overwrite){
    bool abort = false;
    redisAppendCommand(ctx, "EXISTS %" PRIu64, pimghash->hashc);
    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    if (reply->type == REDIS_REPLY_INTEGER){
      if (reply->integer == 1){
		  abort = true;
      }
    }
    freeReplyObject(reply);
    if (abort) return;
  }
  
  redisAppendCommand(ctx, "HMSET %" PRIu64 " %s  %s %s %" PRIu32 " %s %" PRIu32 " %s %" PRIu32,
					 pimghash->hashc,
					 column_names[0].c_str(), pentry->id.c_str(),
					 column_names[1].c_str(), pentry->idnum,
					 column_names[2].c_str(), pentry->seqnum,
					 column_names[3].c_str(), pentry->total);
  r = redisGetReply(ctx,(void**)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);

  redisAppendCommand(ctx, "SELECT 1");
  r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);

  redisAppendCommand(ctx, "LPUSH %s %" PRIu64, pentry->id.c_str(), pimghash->hashc);
  r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);
}

void RedisHashStore::GetHashValue(const struct ImgHash *pimghash, ClipTBLEntry **pentry) {
  redisReply *reply = NULL;

  redisAppendCommand(ctx, "SELECT 0");
  int r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);
  
  redisAppendCommand(ctx, "HGETALL %" PRIu64, pimghash->hashc);
  r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  
  if (reply != NULL && reply->type == REDIS_REPLY_ARRAY){
    if (reply->elements > 0){
      if (*pentry == NULL){
	(*pentry) = new ClipTBLEntry();
      }
      for (int i=0;i < (int)reply->elements;i+=2){
	redisReply *fieldReply = reply->element[i];
	redisReply *valueReply = reply->element[i+1];

	if (fieldReply->type == REDIS_REPLY_STRING && valueReply->type == REDIS_REPLY_STRING){
	  string field(fieldReply->str, fieldReply->len);
	  string value(valueReply->str, valueReply->len);
	  if (field == column_names[0]){
	    ((*pentry)->id) = value;
	  } else if (field == column_names[1]){
	    (*pentry)->idnum = stoi(value);
	  } else if (field == column_names[2]){
	    (*pentry)->seqnum = stoi(value);
	  } else if (field == column_names[3]){
	    (*pentry)->total = stoi(value);
	  }
	}
      }
    }
  }
  freeReplyObject(reply);
}
  

void RedisHashStore::DeleteEntry(const struct ImgHash *pimghash){

  ClipTBLEntry *lookup = NULL;
  GetHashValue(pimghash, &lookup);

  if (lookup != NULL){ // remove from list in DB 1
    redisReply *reply = NULL;
    redisAppendCommand(ctx, "SELECT 0");
    redisAppendCommand(ctx, "DEL %" PRIu64, pimghash->hashc);

    int r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);
    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);
		     
    redisAppendCommand(ctx, "SELECT 1");
    redisAppendCommand(ctx, "LREM %s 0 %" PRIu64 , lookup->id.c_str(), pimghash->hashc);
    
    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);

    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);

    delete lookup;
  }
 
}

void RedisHashStore::DeleteEntry(const string &id) {
  redisAppendCommand(ctx, "SELECT 1");
  redisAppendCommand(ctx, "LRANGE %s 0 -1", id.c_str());
  redisAppendCommand(ctx, "SELECT 0");
  redisReply *reply = NULL, *rangeReply = NULL;

  // receive SELECT 1
  int r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply (r, reply);
  freeReplyObject(reply);

  // receive LRANGE
  r = redisGetReply(ctx, (void**)&rangeReply);
  check_redis_reply(r, rangeReply);

  // receive SELECT 0
  r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);

  
  if (rangeReply != NULL && rangeReply->type == REDIS_REPLY_ARRAY) {
     for (size_t i = 0; i < rangeReply->elements; i++) {
      redisReply *hash = rangeReply->element[i];
      if (hash->type == REDIS_REPLY_STRING) {
	redisAppendCommand(ctx, "DEL %s", hash->str);
	r = redisGetReply(ctx, (void**)&reply);
	check_redis_reply(r, reply);
	freeReplyObject(reply);
      }
    }
    
    redisAppendCommand(ctx, "SELECT 1");
    redisAppendCommand(ctx, "DEL %s", id.c_str());

    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);
    
    r = redisGetReply(ctx, (void**)&reply);
    check_redis_reply(r, reply);
    freeReplyObject(reply);

    freeReplyObject(rangeReply);
  }
  
}

int64_t RedisHashStore::GetCount() {
  redisAppendCommand(ctx, "SELECT 0");
  redisAppendCommand(ctx, "DBSIZE");
  redisReply *reply = NULL;
  
  int r = redisGetReply(ctx, (void **)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);
  
  r = redisGetReply(ctx, (void**) &reply);
  check_redis_reply(r, reply);
  
  int64_t ret = 0;
  if (reply != NULL && reply->type == REDIS_REPLY_INTEGER) {
    ret = reply->integer;
  }
  freeReplyObject(reply);
  return ret;
}

int64_t RedisHashStore::GetCount(const string &id) {
  int64_t ret = -1;
  redisAppendCommand(ctx, "SELECT 1");
  redisAppendCommand(ctx, "LLEN %s", id.c_str());

  redisReply *reply = NULL;
  int r = redisGetReply(ctx, (void **)&reply);
  check_redis_reply(r, reply);
  freeReplyObject(reply);

  r = redisGetReply(ctx, (void**)&reply);
  check_redis_reply(r, reply);
  if (reply != NULL && reply->type == REDIS_REPLY_INTEGER) {
    ret = reply->integer;
  }
  freeReplyObject(reply);
  return ret;
}

vector<string> RedisHashStore::GetEntries() {
  redisAppendCommand(ctx, "SELECT 1");
  redisAppendCommand(ctx, "KEYS *");
  redisReply *reply = NULL;
  int r = redisGetReply(ctx, (void **) &reply);
  check_redis_reply (r, reply);
  freeReplyObject(reply);

  r = redisGetReply(ctx, (void **) &reply);
  check_redis_reply (r, reply);

  vector<string> res;
  if (reply != NULL && reply->type == REDIS_REPLY_ARRAY) {
    res.reserve(reply->elements);
    for (size_t i = 0; i < reply->elements; i++) {
      redisReply *id = reply->element[i];
      if (id->type == REDIS_REPLY_STRING) {
	res.push_back(id->str);
      }
    }
  }
  freeReplyObject(reply);	
  return res;
}

void RedisHashStore::Shutdown() {
  redisFree(ctx);
}
