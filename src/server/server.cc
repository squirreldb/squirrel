// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sofa/pbrpc/pbrpc.h>
#include <iostream>

#include "src/server/server.h"

namespace baidu {
namespace squirrel {
namespace server {

ServerImpl::ServerImpl() : db_(new db::DB()) { }

void ServerImpl::Put(google::protobuf::RpcController* controller,
                     const sdk::PutRequest* request,
                     sdk::PutResponse* response,
                     google::protobuf::Closure* done) {
  if (count_.Get() % 1000000 == 0) { // not for counting, but for heartbeat detect
    SLOG(INFO, "receive put request: %s", request->key().c_str());
  }
  count_.Inc();
  StatusCode status = kOK;
  std::string key = request->key();
  std::string value = request->value();

  db_->Put(key, value, &status);

  response->set_status(status);
  done->Run();
  }

void ServerImpl::Get(google::protobuf::RpcController* controller,
                     const sdk::GetRequest* request,
                     sdk::GetResponse* response,
                     google::protobuf::Closure* done) {
  SLOG(INFO, "receive get request: %s", request->key().c_str());
  std::string value;
  StatusCode status = db_->Get(request->key(), &value);
  response->set_value(value);
  response->set_status(status);
  done->Run();
}

} // namespace server
} // namespace squirrel
} // namespace baidu
