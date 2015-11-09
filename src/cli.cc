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

void GetCallback(std::string* value, baidu::squirrel::StatusCode* status) {
  if (*status == baidu::squirrel::kOK) {
    std::cout << *value << std::endl;
  } else {
    std::cout << StatusCode_Name(*status) << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << "missing arguments" << std::endl;
    return 1;
  }
  std::string op = argv[1];
  std::string key = argv[2];
  std::string value;
  baidu::squirrel::sdk::Client client;
  baidu::squirrel::StatusCode status;
  if (op == "put") {
    value = argv[3];
    client.Put(key, value);
  } else if (op == "get") {
    baidu::squirrel::sdk::UserGetCallback get_callback(boost::bind(&GetCallback, &value, &status));
    client.Get(key, &value, &status, &get_callback);
  }
  return 0;
}
