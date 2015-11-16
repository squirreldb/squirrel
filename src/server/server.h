// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVER_SQUIRREL_SERVER_H
#define SERVER_SQUIRREL_SERVER_H

#include <sofa/pbrpc/pbrpc.h>
#include <counter.h>

#include "src/db/db.h"
#include "src/proto/server_rpc.pb.h"

namespace baidu {
namespace squirrel {
namespace server {

class ServerImpl : public Server {
public:
  ServerImpl();
  virtual ~ServerImpl() {}

private:
  virtual void Put(google::protobuf::RpcController* controller,
                   const PutRequest* request,
                   PutResponse* response,
                   google::protobuf::Closure* done);
  virtual void Get(google::protobuf::RpcController* controller,
                   const GetRequest* request,
                   GetResponse* response,
                   google::protobuf::Closure* done);
  virtual void Delete(google::protobuf::RpcController* controller,
                      const DeleteRequest* request,
                      DeleteResponse* response,
                      google::protobuf::Closure* done);
  virtual void Scan(google::protobuf::RpcController* controller,
                    const ScanRequest* request,
                    ScanResponse* response,
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
