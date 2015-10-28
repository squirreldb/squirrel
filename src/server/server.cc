// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sofa/pbrpc/pbrpc.h>
#include <iostream>
#include "src/proto/server_rpc.pb.h"
#include "src/db/db.h"

extern std::string CONF_server_port;

namespace baidu {
namespace squirrel {
namespace server {

class ServerImpl : public Squirrel::Server {
public:
  ServerImpl() : count(0) {}
  virtual ~ServerImpl() {}

private:
  virtual void Put(google::protobuf::RpcController* controller,
                   const Squirrel::PutRequest* request,
                   Squirrel::PutResponse* response,
                   google::protobuf::Closure* done) {
    if (count % 1000000 == 0) { // not for counting, but for heartbeat detect
      SLOG(INFO, "receive put request: %s", request->key().c_str());
    }
    ++count;
    int status = 0;
    //db_.Put(request->key(), request->value(), request->is_delete(), &status);
    response->set_status(status);
    done->Run();
  }

  virtual void Get(google::protobuf::RpcController* controller,
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

private:
  db::DB db_;
  int count;
};

} //namespace server
} // namespace squirrel
} // namespace baidu

int main() {
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcServerOptions options;
  options.work_thread_num = 4;
  sofa::pbrpc::RpcServer rpc_server(options);

  if (!rpc_server.Start("0.0.0.0:" + CONF_server_port)) {
    SLOG(ERROR, "start server failed");
    return EXIT_FAILURE;
  }

  Squirrel::Server* server_service = new baidu::squirrel::server::ServerImpl();
  if (!rpc_server.RegisterService(server_service)) {
    SLOG(ERROR, "register service failed");
    return EXIT_FAILURE;
  }

  rpc_server.Run();
  rpc_server.Stop();
  return EXIT_SUCCESS;
}
