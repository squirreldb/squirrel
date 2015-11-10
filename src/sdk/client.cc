// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <time.h>
#include <iostream>
#include <string>

#include <boost/bind.hpp>

#include "src/sdk/client.h"

extern std::string CONF_server_addr;
extern std::string CONF_server_port;
extern int CONF_sdk_thread_num;
extern int CONF_sdk_put_pending;

namespace baidu {
namespace squirrel {
namespace sdk {

Client::Client() :
    thread_num_(CONF_sdk_thread_num), thread_pool_(new ThreadPool(thread_num_)) {
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
                         PutResponse* response, StatusCode* status,
                         UserPutCallback* callback, CondVar* cond) {
  if (cntl->Failed()) {
    // SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
    failed_.Inc();
  } else {
    count_.Inc();
  }
  pending_.Dec();
  *status = response->status();

  if (callback) {
    (*callback)(request->key(), request->value(), status);
    delete callback;
  } else {
    cond->Signal();
  }

  delete cntl;
  delete request;
  delete response;
}

void Client::GetCallback(sofa::pbrpc::RpcController* cntl, GetRequest* request,
                         GetResponse* response, std::string* value, StatusCode* status,
                         UserGetCallback* callback, CondVar* cond) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  }
  if (response->status() == kOK) {
    SLOG(INFO, "value: %s", response->value().c_str());
    *value = response->value();
  } else {
    SLOG(ERROR, "not found: %s", request->key().c_str());
    count_.Inc();
  }
  *status = response->status();

  if (callback) {
    (*callback)(request->key(), value, status);
    delete callback;
  } else {
    cond->Signal();
  }

  delete cntl;
  delete request;
  delete response;

}

void Client::DeleteCallback(sofa::pbrpc::RpcController* cntl, DeleteRequest* request,
                            DeleteResponse* response, StatusCode* status,
                            UserDeleteCallback* callback, CondVar* cond) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  }
  if (response->status() == kOK) {
    SLOG(INFO, "key: %s", request->key().c_str());
  } else {
    SLOG(ERROR, "not found: %s", request->key().c_str());
    count_.Inc();
  }
  *status = response->status();

  if (callback) {
    (*callback)(request->key(), status);
    delete callback;
  } else {
    cond->Signal();
  }

  delete cntl;
  delete request;
  delete response;
}

void Client::Put(const std::string& key, const std::string& value, StatusCode* status,
                 UserPutCallback* callback) {
  while (pending_.Get() > CONF_sdk_put_pending) {
    usleep(100);
  }
  PutRequest* request = new PutRequest();
  request->set_key(key);
  request->set_value(value);

  PutResponse* response = new PutResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);

  Mutex mutex;
  CondVar cond(&mutex);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &Client::PutCallback, cntl, request, response,
                            status, callback, &cond);

  pending_.Inc();
  stub_->Put(cntl, request, response, done);
  if (!callback) {
    cond.Wait();
  }
}

void Client::Get(const std::string& key, std::string* value, StatusCode* status,
                 UserGetCallback* callback) {
  GetRequest* request = new GetRequest();
  request->set_key(key);

  GetResponse* response = new GetResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);

  Mutex mutex;
  CondVar cond(&mutex);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &Client::GetCallback, cntl, request, response, value,
                            status, callback, &cond);

  stub_->Get(cntl, request, response, done);
  if (!callback) {
    cond.Wait();
  }
}

void Client::Delete(const std::string& key, StatusCode* status, UserDeleteCallback* callback) {
  DeleteRequest* request = new DeleteRequest();
  request->set_key(key);

  DeleteResponse* response = new DeleteResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);

  Mutex mutex;
  CondVar cond(&mutex);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &Client::DeleteCallback, cntl, request, response,
                            status, callback, &cond);

  stub_->Delete(cntl, request, response, done);
  if (!callback) {
    cond.Wait();
  }
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
