// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SQUIRREL_CLIENT_H
#define CLIENT_SQUIRREL_CLIENT_H

#include <sofa/pbrpc/pbrpc.h>
#include <thread_pool.h>
#include <mutex.h>
#include <counter.h>

#include "src/proto/server_rpc.pb.h"
#include "src/proto/status_code.pb.h"

namespace baidu {
namespace squirrel {
namespace sdk {

typedef boost::function<void (const std::string&, const std::string&, StatusCode*)> UserPutCallback;
typedef boost::function<void (const std::string&, std::string*, StatusCode*)> UserGetCallback;
typedef boost::function<void (const std::string&, StatusCode*)> UserDeleteCallback;

typedef server::PutRequest PutRequest;
typedef server::PutResponse PutResponse;
typedef server::GetRequest GetRequest;
typedef server::GetResponse GetResponse;
typedef server::DeleteRequest DeleteRequest;
typedef server::DeleteResponse DeleteResponse;
typedef server::Server_Stub Server_Stub;

class Client
{
public:
  Client();
  ~Client();
  void init();

  void Put(const std::string& key, const std::string& value, StatusCode* status,
           UserPutCallback* callback);
  void Get(const std::string& key, std::string* value, StatusCode* status,
           UserGetCallback* callback);
  void Delete(const std::string& key, StatusCode* status, UserDeleteCallback* callback);

  void Scan(const std::string& key, const std::string& value, StatusCode* status);

  void GetStat(int* count, int* failed, int* pending, int* thread_pool_pendin, std::string* str);
  void ResetStat();


private:
  void PutCallback(sofa::pbrpc::RpcController* cntl, PutRequest* request,
                   PutResponse* response, StatusCode* status,
                   UserPutCallback* callback, CondVar* cond);
  void GetCallback(sofa::pbrpc::RpcController* cntl, GetRequest* request,
                   GetResponse* response, std::string* value, StatusCode* status,
                   UserGetCallback* callback, CondVar* cond);
  void DeleteCallback(sofa::pbrpc::RpcController* cntl, DeleteRequest* request,
                      DeleteResponse* response, StatusCode* status,
                      UserDeleteCallback* callback, CondVar* cond);

private:
  int thread_num_;
  common::Counter count_;
  common::Counter failed_;
  common::Counter pending_;
  ThreadPool* thread_pool_;

  sofa::pbrpc::RpcClient* rpc_client_;
  sofa::pbrpc::RpcChannel* rpc_channel_;
  Server_Stub* stub_;
};

} // namespace sdk
} // squirrel
} // namespace baidu

#endif // CLIENT_SQUIRREL_CLIENT_H
