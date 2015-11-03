// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sofa/pbrpc/pbrpc.h>
#include <iostream>

#include "src/server/server.h"

namespace baidu {
namespace squirrel {
namespace server {

void ServerImpl::Put(google::protobuf::RpcController* controller,
                     const Squirrel::PutRequest* request,
                     Squirrel::PutResponse* response,
                     google::protobuf::Closure* done) {
  if (count_.Get() % 1000000 == 0) { // not for counting, but for heartbeat detect
    SLOG(INFO, "receive put request: %s", request->key().c_str());
  }
  count_.Inc();
  int status = 0;
  db_.Put(request->key(), request->value(), &status);
  response->set_status(status);
  done->Run();
  }

void ServerImpl::Get(google::protobuf::RpcController* controller,
                     const Squirrel::GetRequest* request,
                     Squirrel::GetResponse* response,
                     google::protobuf::Closure* done) {
  SLOG(INFO, "receive get request: %s", request->key().c_str());
  std::string value;
  int status = 0;
  db_.Get(request->key(), &value, &status);
  response->set_value(value);
  response->set_status(status);
  done->Run();
  }

} // namespace server
} // namespace squirrel
} // namespace baidu
