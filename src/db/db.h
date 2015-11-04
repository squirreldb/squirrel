// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_DB_H_
#define SQUIRREL_DB_H_

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include <mutex.h>

#include "src/proto/status_code.pb.h"
#include "index_db.h"

namespace baidu {
namespace squirrel {
namespace db {

class DB {
public:
  DB();
  void Put(const std::string& key, const std::string& value, StatusCode* status);
  void Get(const std::string& key, std::string* value, StatusCode* status);

private:
  Mutex mutex_;
  uint64_t file_num_;
  std::string filename_;
  std::ofstream* fout_;
  uint32_t offset_;

  IndexDB* index_;
};

} // namespace db
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_DB_H_
