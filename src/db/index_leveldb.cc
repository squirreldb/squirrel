// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include <leveldb/status.h>
#include <leveldb/slice.h>

#include "index_leveldb.h"
#include "src/util/utils.h"

namespace baidu {
namespace squirrel {
namespace db {


IndexLevelDB::IndexLevelDB(const std::string& dbname) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, dbname, &index_);
  if (status.ok()) {
    std::cerr << "index_db init OK\n";
  } else {
    std::cerr << status.ToString() << std::endl;
  }
}

StatusCode IndexLevelDB::Put(const std::string& key, const std::string& value) {
  //std::cerr << "put: " << meta->ToString() << std::endl;
  leveldb::WriteOptions options;
  index_->Put(options, key, value);
  return kOK;
}

StatusCode IndexLevelDB::Get(const std::string& key, std::string* value) {
  leveldb::ReadOptions options;
  leveldb::Status status = index_->Get(options, key, value);
  if (status.IsNotFound()) {
    return kKeyNotFound;
  }
  return kOK;
}

StatusCode IndexLevelDB::Delete(const std::string& key) {
  //std::cerr << "delete: " << key << std::endl;
  leveldb::WriteOptions options;
  index_->Delete(options, key);

  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
