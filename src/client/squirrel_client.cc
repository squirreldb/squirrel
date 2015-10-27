// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <string>
#include <sofa/pbrpc/pbrpc.h>
#include <boost/bind.hpp>

#include "src/proto/squirrel_rpc.pb.h"
#include "src/client/squirrel_client.h"

SquirrelClient::SquirrelClient() :
    count_(0), failed_(0), thread_num_(4), pending_(0),
    thread_pool_(new baidu::common::ThreadPool(thread_num_)) {
  pthread_mutex_init(&mutex_, NULL);
  init();
}

SquirrelClient::~SquirrelClient() {}

void SquirrelClient::init() {
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcClientOptions options;
  options.work_thread_num = 8;
  options.callback_thread_num = 8;
  options.max_pending_buffer_size = 4;

  rpc_client_ = new sofa::pbrpc::RpcClient(options);
  rpc_channel_ = new sofa::pbrpc::RpcChannel(rpc_client_, "st01-spi-session1.st01.baidu.com:11221");
  stub_ = new Squirrel::SquirrelServer_Stub(rpc_channel_);
}

void SquirrelClient::PutCallback(sofa::pbrpc::RpcController* cntl,
                                 Squirrel::PutRequest* request,
                                 Squirrel::PutResponse* response) {
  if (cntl->Failed()) {
    //SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
    pthread_mutex_lock(&mutex_);
    failed_ += 1;
    pthread_mutex_unlock(&mutex_);
  } else {
    pthread_mutex_lock(&mutex_);
    count_ += 1;
    pending_ -= 1;
    pthread_mutex_unlock(&mutex_);
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
    pthread_mutex_lock(&mutex_);
    count_ += 1;
    pthread_mutex_unlock(&mutex_);
  }

  delete cntl;
  delete request;
  delete response;
}

void SquirrelClient::Put(const std::string& key, const std::string& value, const bool is_delete) {
  baidu::common::ThreadPool::Task task =
      boost::bind(&SquirrelClient::DoPut, this, key, value, is_delete);
  thread_pool_->AddTask(task);
}

void SquirrelClient::DoPut(const std::string& key, const std::string& value, const bool is_delete) {
  if (pending_ > 1000) {
    usleep(1);
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

  pending_ += 1;
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

void SquirrelClient::GetStat(int* count, int* failed, int* pending) {
  pthread_mutex_lock(&mutex_);
  *count = count_;
  *failed = failed_;
  *pending = pending_;
  pthread_mutex_unlock(&mutex_);
}

void SquirrelClient::ResetStat() {
  pthread_mutex_lock(&mutex_);
  count_ = 0;
  failed_ = 0;
  pending_ = 0;
  pthread_mutex_unlock(&mutex_);
}

void test_put(SquirrelClient* client, std::string& key, std::string& value, bool is_delete) {
  while (true) {
    client->Put(key, value, is_delete);
  }
}

int main(int argc, char * argv[]) {
  struct timeval tv_start, tv_end;
  gettimeofday(&tv_start, NULL);

  SquirrelClient client;
  // client.Put("k", "v", false);
  std::string key = "k";
  std::string value = "v";
  baidu::common::ThreadPool thread_pool;
  for (int i = 0; i < 2; ++i) {
    baidu::common::ThreadPool::Task task =
        boost::bind(&test_put, &client, key, value, false);
    thread_pool.AddTask(task);
  }

  while (true) {
    gettimeofday(&tv_end, NULL);
    long start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    long end = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
    double interval = (end - start) / double(1000000);

    int count, failed, pending;
    client.GetStat(&count, &failed, &pending);
    std::cout << "Qps=" << int((count) / interval)
              << "\tfailed=" << int(double(failed) / interval)
              << "\tpending=" << int(double(pending) / interval)
              << "\tinterval=" << interval << std::endl;
    client.ResetStat();

    tv_start = tv_end;
    sleep(1);
  }

  return EXIT_SUCCESS;
}
