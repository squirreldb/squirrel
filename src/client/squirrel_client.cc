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
#include "src/common/thread_pool.h"

int count = 0;
int failed = 0;
int thread_num = 4;
int pending = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct PutArgs {
  Squirrel::SquirrelServer_Stub* stub;
  std::string key;
  std::string value;
  bool is_delete;
};

void PutCallback(sofa::pbrpc::RpcController* cntl,
                 Squirrel::PutRequest* request,
                 Squirrel::PutResponse* response) {
  if (cntl->Failed()) {
    //SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
    pthread_mutex_lock(&mutex);
    failed += 1;
    pthread_mutex_unlock(&mutex);
  } else {
    pthread_mutex_lock(&mutex);
    count += 1;
    pending -= 1;
    pthread_mutex_unlock(&mutex);
  }

  delete cntl;
  delete request;
  delete response;
}

void GetCallback(sofa::pbrpc::RpcController* cntl,
                 Squirrel::GetRequest* request,
                 Squirrel::GetResponse* response) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  }
  if (response->status() == 0) {
    SLOG(INFO, "value: %s", response->value().c_str());
  } else {
    pthread_mutex_lock(&mutex);
    count += 1;
    pthread_mutex_unlock(&mutex);
  }

  delete cntl;
  delete request;
  delete response;
}

void Put(Squirrel::SquirrelServer_Stub* stub, std::string key, std::string value, bool is_delete) {
  while (true) {
    pthread_mutex_lock(&mutex);
    if (pending > 1000) {
      usleep(1);
    }
    pthread_mutex_unlock(&mutex);
    Squirrel::PutRequest* request = new Squirrel::PutRequest();
    request->set_key(key);
    request->set_value(value);
    request->set_is_delete(is_delete);

    Squirrel::PutResponse* response = new Squirrel::PutResponse();
    sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);
    google::protobuf::Closure* done = sofa::pbrpc::NewClosure(&PutCallback, cntl, request, response);

    stub->Put(cntl, request, response, done);
    pending += 1;
  }
}

void Get(Squirrel::SquirrelServer_Stub* stub, std::string key) {
  Squirrel::GetRequest* request = new Squirrel::GetRequest();
  request->set_key(key);

  Squirrel::GetResponse* response = new Squirrel::GetResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done = sofa::pbrpc::NewClosure(&GetCallback, cntl, request, response);

  stub->Get(cntl, request, response, done);
}

int main(int argc, char * argv[]) {
  if (argc < 3) {
    std::cout << "Invalid argument number: " << argc << std::endl;
    return 1;
  }

  // rpc init
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcClientOptions options;
  options.work_thread_num = 8;
  options.callback_thread_num = 8;
  options.max_pending_buffer_size = 4;

  sofa::pbrpc::RpcClient rpc_client(options);

  sofa::pbrpc::RpcChannel rpc_channel(&rpc_client, "st01-spi-session1.st01.baidu.com:11221");
  Squirrel::SquirrelServer_Stub stub(&rpc_channel);

  std::string op = argv[1];
  std::string key = argv[2];
  std::string value;

  struct timeval tv_start, tv_end;
  gettimeofday(&tv_start, NULL);

  std::vector<pthread_t> threads;
  pthread_mutex_init(&mutex, NULL);

  ThreadPool* thread_pool = new ThreadPool(8);
  for (int i = 0; i < 8; ++i) {
    ThreadPool::Task task =
        boost::bind(&Put, &stub, key, value, false);
    thread_pool->AddTask(task);
  }

  while (true) {
    gettimeofday(&tv_end, NULL);
    long start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    long end = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
    double interval = (end - start) / double(1000000);
    std::cout << "interval = " << interval << std::endl;

    pthread_mutex_lock(&mutex);
    std::cout << "Qps=" << int((count) / interval)
              << "\tfailed=" << int(double(failed) / interval)
              << "\tpending=" << int(double(pending) / interval)
              << "\tinterval=" << interval << std::endl;
    count = 0;
    failed = 0;
    pthread_mutex_unlock(&mutex);

    tv_start = tv_end;
    sleep(1);
  }

  return EXIT_SUCCESS;
}
