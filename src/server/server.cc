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
  // write data into database
  uint32_t offset = 0;
  std::string filename;
  std::string key = request->key();
  std::string value = request->value();
  uint32_t length = key.length() + value.length() + 8;
  std::cerr << "total=" << length << std::endl;
  db_.Put(key, key.length(), value, value.length(), &offset, &filename, &status);

  // write index
  EntryMeta meta = {filename, offset, length};
  MutexLock lock(&mutex_);
  index_[key] = meta;
  std::map<std::string, EntryMeta>::iterator it = index_.begin();
  std::cerr << key << ":" << meta.filename << "-" << meta.offset << "-" << meta.length << std::endl;
  mutex_.Unlock();

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
