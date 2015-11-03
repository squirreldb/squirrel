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

namespace baidu {
namespace squirrel {
namespace sdk {

class Client
{
public:
  Client();
  ~Client();
  void init();

  void Put(const std::string& key, const std::string& value);
  void Get(const std::string& key, std::string* value);
  void Delete(const std::string& key, int32_t* status);

  void GetStat(int* count, int* failed, int* pending, int* thread_pool_pendin, std::string* str);
  void ResetStat();

private:
  void PutCallback(sofa::pbrpc::RpcController* cntl, Squirrel::PutRequest* request,
                   Squirrel::PutResponse* response);
  void GetCallback(sofa::pbrpc::RpcController* cntl, Squirrel::GetRequest* request,
                   Squirrel::GetResponse* response);
  void DeleteCallback(sofa::pbrpc::RpcController* cntl, Squirrel::DeleteRequest* request,
                      Squirrel::DeleteResponse* response);

private:
  int thread_num_;
  common::Counter count_;
  common::Counter failed_;
  common::Counter pending_;
  ThreadPool* thread_pool_;

  sofa::pbrpc::RpcClient* rpc_client_;
  sofa::pbrpc::RpcChannel* rpc_channel_;
  Squirrel::Server_Stub* stub_;
};

} // namespace sdk
} // squirrel
} // namespace baidu

#endif // CLIENT_SQUIRREL_CLIENT_H
