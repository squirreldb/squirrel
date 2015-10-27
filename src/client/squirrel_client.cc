// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <string>

#include <sofa/pbrpc/pbrpc.h>
#include <boost/bind.hpp>
#include <thread.h>

#include "src/proto/squirrel_rpc.pb.h"
#include "src/client/squirrel_client.h"

namespace baidu {
namespace squirrel {
namespace sdk {

SquirrelClient::SquirrelClient() :
    thread_num_(4), thread_pool_(new ThreadPool(thread_num_)) {
  init();
}

SquirrelClient::~SquirrelClient() {}

void SquirrelClient::init() {
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcClientOptions options;

  // options.work_thread_num = 8;
  // options.callback_thread_num = 8;
  // options.max_pending_buffer_size = 4;

  rpc_client_ = new sofa::pbrpc::RpcClient(options);
  rpc_channel_ = new sofa::pbrpc::RpcChannel(rpc_client_, "st01-spi-session1.st01.baidu.com:8221");
  stub_ = new Squirrel::SquirrelServer_Stub(rpc_channel_);
}

void SquirrelClient::PutCallback(sofa::pbrpc::RpcController* cntl,
                                 Squirrel::PutRequest* request,
                                 Squirrel::PutResponse* response) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
    failed_.Inc();
  } else {
    pending_.Dec();
    count_.Inc();
  }

  delete cntl;
  delete request;
  delete response;
}

void SquirrelClient::GetCallback(sofa::pbrpc::RpcController* cntl,
                                 Squirrel::GetRequest* request,
                                 Squirrel::GetResponse* response) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  }
  if (response->status() == 0) {
    SLOG(INFO, "value: %s", response->value().c_str());
  } else {
    count_.Inc();
  }

  delete cntl;
  delete request;
  delete response;
}

void SquirrelClient::Put(const std::string& key, const std::string& value, const bool is_delete) {
  ThreadPool::Task task =
      boost::bind(&SquirrelClient::DoPut, this, key, value, is_delete);
  thread_pool_->AddTask(task);
}

void SquirrelClient::DoPut(const std::string& key, const std::string& value, const bool is_delete) {
  if (pending_.Get() > 1000) {
    usleep(3);
  }
  Squirrel::PutRequest* request = new Squirrel::PutRequest();
  request->set_key(key);
  request->set_value(value);
  request->set_is_delete(is_delete);

  Squirrel::PutResponse* response = new Squirrel::PutResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &SquirrelClient::PutCallback, cntl, request, response);

  pending_.Inc();
  stub_->Put(cntl, request, response, done);

}

void SquirrelClient::Get(std::string key) {
  Squirrel::GetRequest* request = new Squirrel::GetRequest();
  request->set_key(key);

  Squirrel::GetResponse* response = new Squirrel::GetResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done =
    sofa::pbrpc::NewClosure(this, &SquirrelClient::GetCallback, cntl, request, response);

  stub_->Get(cntl, request, response, done);
}

void SquirrelClient::GetStat(int* count, int* failed, int* pending, int* thread_pool_pending, std::string* str) {
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

void SquirrelClient::ResetStat() {
  count_.Set(0);
  failed_.Set(0);
  pending_.Set(0);
}

} // namespace sdk
} // namespace squirrel
} // namespace baidu

void test_put(baidu::squirrel::sdk::SquirrelClient* client, std::string& key,
              std::string& value, bool is_delete) {
  while (true) {
    int thread_pool_pending;
    client->GetStat(NULL, NULL, NULL, &thread_pool_pending, NULL);
    if (thread_pool_pending > 100) {
      usleep(5000);
    }
    client->Put(key, value, is_delete);
  }
}

int main(int argc, char * argv[]) {
  struct timeval tv_start, tv_end;
  gettimeofday(&tv_start, NULL);

  baidu::squirrel::sdk::SquirrelClient client;
  // client.Put("k", "v", false);
  std::string key = "k";
  std::string value = "v";

  baidu::common::Thread thread;
  thread.Start(boost::bind(&test_put, &client, key, value, false));

  while (true) {
    gettimeofday(&tv_end, NULL);
    long start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    long end = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
    double interval = (end - start) / double(1000000);

    int count, failed, pending, thread_pool_pending;
    std::string profile_str;
    client.GetStat(&count, &failed, &pending, &thread_pool_pending, &profile_str);
    std::cout << "Qps=" << int((count) / interval)
              << "\tfailed=" << int(double(failed) / interval)
              << "\tpending=" << int(double(pending) / interval)
              << "\tinterval=" << interval
              << "\tthreadpool_pending=" << thread_pool_pending
              << "\ttp=" << profile_str
              << std::endl;
    client.ResetStat();

    tv_start = tv_end;
    sleep(1);
  }

  thread.Join();
  return EXIT_SUCCESS;
}
