// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_SQUIRREL_SERVER_H
#define SERVER_SQUIRREL_SERVER_H

#include <string>
#include <map>

#include <sofa/pbrpc/pbrpc.h>
#include <counter.h>
#include <mutex.h>

#include "src/db/db.h"
#include "src/proto/server_rpc.pb.h"

namespace baidu {
namespace squirrel {
namespace server {

class ServerImpl : public sdk::Server {
public:
  ServerImpl();
  virtual ~ServerImpl() {}

private:
  virtual void Put(google::protobuf::RpcController* controller,
                   const sdk::PutRequest* request,
                   sdk::PutResponse* response,
                   google::protobuf::Closure* done);
  virtual void Get(google::protobuf::RpcController* controller,
                   const sdk::GetRequest* request,
                   sdk::GetResponse* response,
                   google::protobuf::Closure* done);

private:
  Mutex mutex_;
  common::Counter count_;
  db::DB* db_;
};

} // namespace server
} // namespace squirrel
} // namespace baidu

#endif // SERVER_SQUIRREL_SERVER_H
