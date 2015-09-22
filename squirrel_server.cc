// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sofa/pbrpc/pbrpc.h>
#include <iostream>
#include "squirrel_rpc.pb.h"
#include "dummy_db.h"

class DummyDBImpl : public Squirrel::SquirrelServer {
public:
  DummyDBImpl() {
    db_ = DummyDB();
  }
  virtual ~DummyDBImpl() {}

private:
  virtual void Put(google::protobuf::RpcController* controller,
                   const Squirrel::PutRequest* request,
                   Squirrel::PutResponse* response,
                   google::protobuf::Closure* done) {
    SLOG(INFO, "receive put request: %s", request->key().c_str());
    int status = 0;
    db_.Put(request->key(), request->value(), request->is_delete(), &status);
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
  DummyDB db_;
};

int main() {
  SOFA_PBRPC_SET_LOG_LEVEL(INFO);

  sofa::pbrpc::RpcServerOptions options;
  options.work_thread_num = 4;
  sofa::pbrpc::RpcServer rpc_server(options);

  if (!rpc_server.Start("0.0.0.0:11220")) {
      SLOG(ERROR, "start server failed");
      return EXIT_FAILURE;
  }

  Squirrel::SquirrelServer* dummy_db_service = new DummyDBImpl();
  if (!rpc_server.RegisterService(dummy_db_service)) {
      SLOG(ERROR, "register service failed");
      return EXIT_FAILURE;
  }

  rpc_server.Run();
  rpc_server.Stop();
  return EXIT_SUCCESS;
}
