// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include <leveldb/status.h>
#include <leveldb/slice.h>

#include "index_db.h"
#include "src/util/utils.h"

namespace baidu {
namespace squirrel {
namespace db {

IndexDB::IndexDB(const std::string& dbname) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, dbname, &index_);
  if (status.ok()) {
    std::cerr << "index_db init OK\n";
  } else {
    std::cerr << status.ToString() << std::endl;
  }
}

StatusCode IndexDB::Put(const std::string& key, EntryMeta* meta) {
  //std::cerr << "put: " << meta->ToString() << std::endl;
  char buf[8];
  EncodeFixed32(buf, meta->offset);
  EncodeFixed32(buf + 4, meta->length);
  std::string value = meta->filename;
  value.append(buf, 8);
  leveldb::Slice value_slice(value);
  leveldb::WriteOptions options;
  index_->Put(options, key, value_slice);
  return kOK;
}

StatusCode IndexDB::Get(const std::string& key, EntryMeta* meta) {
  std::string value;
  leveldb::ReadOptions options;
  leveldb::Status status = index_->Get(options, key, &value);
  if (status.IsNotFound()) {
    return kKeyNotFound;
  }
  uint32_t offset;
  size_t value_size = value.size();
  memcpy(&offset, &value[value_size - 8], sizeof(offset));
  meta->offset = offset;
  uint32_t length;
  memcpy(&length, &value[value_size - 4], sizeof(length));
  meta->length = length;
  meta->filename.assign(value, 0, value_size - 8);
  return kOK;
}

StatusCode IndexDB::Delete(const std::string& key) {
  //std::cerr << "delete: " << key << std::endl;
  leveldb::WriteOptions options;
  index_->Delete(options, key);

  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
