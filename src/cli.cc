// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <unistd.h>

#include <thread.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include "src/proto/status_code.pb.h"
#include "src/sdk/client.h"

namespace baidu {
namespace squirrel {

void GetCallback(const std::string& key, std::string* value, StatusCode* status) {
  if (*status == kOK) {
    std::cout << key << "-" << *value << std::endl;
  } else {
    std::cout << StatusCode_Name(*status) << std::endl;
  }
}

int32_t GetOp(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "missing arguments" << std::endl;
    return 1;
  }
  std::string key = argv[2];
  sdk::Client client;
  StatusCode status;
  std::string value;

  // async
  // sdk::UserGetCallback get_callback(boost::bind(&GetCallback, _1, _2, _3));
  // client.Get(key, &value, &status, &get_callback);

  client.Get(key, &value, &status, NULL);
  if (status == kOK) {
    std::cout << value << std::endl;
  } else {
    std::cout << StatusCode_Name(status) << std::endl;
  }
  return 0;
}

int32_t PutOp(int argc, char* argv[]) {
  if (argc < 4) {
    std::cerr << "missing arguments" << std::endl;
    return 1;
  }
  std::string key = argv[2];
  std::string value = argv[3];
  sdk::Client client;
  StatusCode status;

  client.Put(key, value, &status, NULL);
  if (status == kOK) {
    std::cout << "Put " << key << "-" << value << std::endl;
  } else {
    std::cout << StatusCode_Name(status) << std::endl;
  }
  return 0;
}

int32_t DeleteOp(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "missing arguments" << std::endl;
    return 1;
  }
  std::string key = argv[2];
  sdk::Client client;
  StatusCode status;

  client.Delete(key, &status, NULL);
  if (status == kOK) {
    std::cout << "Delete " << key << std::endl;
  } else {
    std::cout << StatusCode_Name(status) << std::endl;
  }
  return 0;
}

} // namespace sdk
} // namespace squirrel

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "missing arguments" << std::endl;
    return 1;
  }
  std::string op = argv[1];
  if (op == "put") {
    return baidu::squirrel::PutOp(argc, argv);
  } else if (op == "get") {
    return baidu::squirrel::GetOp(argc, argv);
  } else if (op == "del") {
    return baidu::squirrel::DeleteOp(argc, argv);
  }
  return 0;
}
