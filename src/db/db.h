// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_DB_H_
#define SQUIRREL_DB_H_

#include <fstream>

#include "src/proto/status_code.pb.h"
#include "src/proto/server_rpc.pb.h"
#include "index_db.h"


namespace baidu {
namespace squirrel {
namespace db {

typedef ::google::protobuf::RepeatedPtrField<baidu::squirrel::server::KvPair> KvPairResults;
typedef baidu::squirrel::server::KvPair KvPair;

class DB {
public:
  DB();
  StatusCode Put(const std::string& key, const std::string& value);
  StatusCode Get(const std::string& key, std::string* value);
  StatusCode Delete(const std::string& key);
  StatusCode Scan(const std::string& start, const std::string& end, KvPairResults* results,
                  bool* complete);

private:
  void Recover();
  void LogFileNum();
  StatusCode SwitchFile();

private:
  Mutex mutex_;
  uint32_t file_num_;
  std::string filename_;
  int fout_;
  uint32_t offset_;
  const uint32_t file_size_limit_;

  IndexDB* index_;
};

} // namespace db
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_DB_H_
