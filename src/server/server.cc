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
                     const PutRequest* request,
                     PutResponse* response,
                     google::protobuf::Closure* done) {
  if (count_.Get() % 1000000 == 0) {
    //SLOG(INFO, "receive put request: %s %s", request->key().c_str(),
         //request->value().c_str());
  }
  count_.Inc();
  std::string key = request->key();
  std::string value = request->value();

  StatusCode status = db_->Put(key, value);

  response->set_status(status);
  done->Run();
  }

void ServerImpl::Get(google::protobuf::RpcController* controller,
                     const GetRequest* request,
                     GetResponse* response,
                     google::protobuf::Closure* done) {
  SLOG(INFO, "receive get request: %s", request->key().c_str());
  std::string value;
  StatusCode status = db_->Get(request->key(), &value);
  response->set_value(value);
  response->set_status(status);
  done->Run();
}

void ServerImpl::Delete(google::protobuf::RpcController* controller,
                        const DeleteRequest* request,
                        DeleteResponse* response,
                        google::protobuf::Closure* done) {
  SLOG(INFO, "receive get request: %s", request->key().c_str());
  StatusCode status = db_->Delete(request->key());
  response->set_status(status);
  done->Run();
}

} // namespace server
} // namespace squirrel
} // namespace baidu
