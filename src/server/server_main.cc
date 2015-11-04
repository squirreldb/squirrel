// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/server/server.h"

extern std::string CONF_server_port;

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
