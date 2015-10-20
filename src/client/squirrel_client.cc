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
#include "src/proto/squirrel_rpc.pb.h"

int count = 0;
int failed = 0;
int thread_num = 4;
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

void* Put(void* args) {
  PutArgs* put_args = static_cast<PutArgs*>(args);
  for (int i = 0; ; ++i) {
    Squirrel::PutRequest* request = new Squirrel::PutRequest();
    request->set_key(put_args->key);
    request->set_value(put_args->value);
    request->set_is_delete(put_args->is_delete);

    Squirrel::PutResponse* response = new Squirrel::PutResponse();
    sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
    cntl->SetTimeout(3000);
    google::protobuf::Closure* done = sofa::pbrpc::NewClosure(&PutCallback, cntl, request, response);

    put_args->stub->Put(cntl, request, response, done);
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

  struct timeval tv_start;
  gettimeofday(&tv_start, NULL);

  std::vector<pthread_t> threads;
  pthread_mutex_init(&mutex, NULL);
  if (op == "put") {
    value = argv[3];
    std::cout << op << " : " << key << "--" << value << std::endl;
    for (int i = 0; i < thread_num; ++i) {
      pthread_t ntid;
      PutArgs args;
      args.stub = &stub;
      args.key = key;
      args.value = value;
      args.is_delete = false;
      int err = pthread_create(&ntid, NULL, Put, static_cast<void*>(&args));
      if (err != 0) {
        // SLOG(ERROR, "create thread failed: %s", strerror(err));
      } else {
        std::cout << "started thread #" << i << std::endl;
        threads.push_back(ntid);
      }
    }
  } else {
    std::cout << op << " : " << key << std::endl;
    Get(&stub, key);
  }

  while (true) {
    pthread_mutex_lock(&mutex);
    std::cout << "Qps=" << count << " failed=" << failed << std::endl;
    count = 0;
    failed = 0;
    pthread_mutex_unlock(&mutex);
    sleep(1);
  }
  for (int i = 0; i < thread_num; ++i) {
    pthread_join(threads[i], NULL);
  }
  std::cout << "joined" << std::endl;

  return EXIT_SUCCESS;
}
