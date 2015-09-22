// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <string>
#include <sofa/pbrpc/pbrpc.h>
#include "squirrel_rpc.pb.h"

int count = 0;

void PutCallback(sofa::pbrpc::RpcController* cntl,
                 Squirrel::PutRequest* request,
                 Squirrel::PutResponse* response) {
  if (cntl->Failed()) {
    SLOG(ERROR, "rpc failed: %s", cntl->ErrorText().c_str());
  } else {
    SLOG(INFO, "resp status: %d", response->status());
    count += 1;
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
    SLOG(INFO, "key: %s not found", request->key().c_str());
    count += 1;
  }

  delete cntl;
  delete request;
  delete response;
}

void Put(Squirrel::SquirrelServer_Stub* stub, std::string key, std::string value, bool is_delete) {
  Squirrel::PutRequest* request = new Squirrel::PutRequest();
  request->set_key(key);
  request->set_value(value);
  request->set_is_delete(is_delete);

  Squirrel::PutResponse* response = new Squirrel::PutResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done = sofa::pbrpc::NewClosure(&PutCallback, cntl, request, response);
  stub->Put(cntl, request, response, done);
  sleep(1);
}

void Get(Squirrel::SquirrelServer_Stub* stub, std::string key) {
  Squirrel::GetRequest* request = new Squirrel::GetRequest();
  request->set_key(key);

  Squirrel::GetResponse* response = new Squirrel::GetResponse();
  sofa::pbrpc::RpcController* cntl = new sofa::pbrpc::RpcController();
  cntl->SetTimeout(3000);
  google::protobuf::Closure* done = sofa::pbrpc::NewClosure(&GetCallback, cntl, request, response);
  stub->Get(cntl, request, response, done);
  sleep(1);
}

int main(int argc, char * argv[]) {
  if (argc < 3) {
    std::cout << "Invalid argument number: " << argc << std::endl;
    return 1;
  }

  // rpc init
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcClientOptions options;
  sofa::pbrpc::RpcClient rpc_client(options);

  sofa::pbrpc::RpcChannel rpc_channel(&rpc_client, "st01-spi-session0.st01.baidu.com:11220");
  Squirrel::SquirrelServer_Stub stub(&rpc_channel);

  std::string op = argv[1];
  std::string key = argv[2];
  std::string value;
  if (op == "put") {
    value = argv[3];
    std::cout << op << " : " << key << "--" << value << std::endl;
    for (int i = 0; i < 100; ++i) {
      std::cout << i << std::endl;
      Put(&stub, key, value, false);
    }
  } else {
    std::cout << op << " : " << key << std::endl;
    Get(&stub, key);
  }
  while (count != 100) {
    sleep(1);
  }
  
  return EXIT_SUCCESS;
}
