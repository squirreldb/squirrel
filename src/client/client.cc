// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <time.h>
#include <iostream>
#include <string>

#include <boost/bind.hpp>

#include "src/client/client.h"

extern std::string CONF_server_addr;
extern std::string CONF_server_port;
extern int CONF_thread_num;
extern int CONF_put_pending;

namespace baidu {
namespace squirrel {
namespace sdk {

Client::Client() :
    thread_num_(CONF_thread_num), thread_pool_(new ThreadPool(thread_num_)) {
  init();
}

Client::~Client() {}

void Client::init() {
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcClientOptions options;
  // options.work_thread_num = 4;
  // options.callback_thread_num = 4;
  rpc_client_ = new sofa::pbrpc::RpcClient(options);
  rpc_channel_ = new sofa::pbrpc::RpcChannel(rpc_client_, CONF_server_addr + ":" + CONF_server_port);
  stub_ = new Server_Stub(rpc_channel_);
}

void Client::PutCallback(sofa::pbrpc::RpcController* cntl, PutRequest* request,
                         PutResponse* response) {
  if (cntl->Failed()) {
    // SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
    failed_.Inc();
  } else {
    count_.Inc();
  }
  pending_.Dec();

  delete cntl;
  delete request;
  delete response;
}

void Client::GetCallback(sofa::pbrpc::RpcController* cntl, GetRequest* request,
                         GetResponse* response) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  }
  if (response->status() == kOK) {
    SLOG(INFO, "value: %s", response->value().c_str());
  } else {
    count_.Inc();
  }

  delete cntl;
  delete request;
  delete response;
}

void Client::DeleteCallback(sofa::pbrpc::RpcController* cntl, DeleteRequest* request,
                            DeleteResponse* response) {
  // TODO
}

void Client::Put(const std::string& key, const std::string& value) {
  while (pending_.Get() > CONF_put_pending) {
    usleep(1000);
  }
  PutRequest* request = new PutRequest();
  request->set_key(key);
  request->set_value(value);

  PutResponse* response = new PutResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &Client::PutCallback, cntl, request, response);

  pending_.Inc();
  ThreadPool::Task task =
    boost::bind(&Server_Stub::Put, stub_, cntl, request, response, done);
  thread_pool_->AddTask(task);
}

void Client::Get(const std::string& key, std::string* value) {
  GetRequest* request = new GetRequest();
  request->set_key(key);

  GetResponse* response = new GetResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &Client::GetCallback, cntl, request, response);

  stub_->Get(cntl, request, response, done);
}

void Client::Delete(const std::string& key, StatusCode* status) {
  // TODO
}

void Client::GetStat(int* count, int* failed, int* pending,
                             int* thread_pool_pending, std::string* str) {
  if (count) {
    *count = count_.Get();
  }
  if (failed) {
    *failed = failed_.Get();
  }
  if (pending) {
    *pending = pending_.Get();
  }
  if (thread_pool_pending) {
    *thread_pool_pending = thread_pool_->PendingNum();
  }
  if (str) {
    *str = thread_pool_->ProfilingLog();
  }
}

void Client::ResetStat() {
  count_.Set(0);
  failed_.Set(0);
}

} // namespace sdk
} // namespace squirrel
} // namespace baidu
