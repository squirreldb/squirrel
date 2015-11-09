// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "index_db.h"

namespace baidu {
namespace squirrel {
namespace db {

IndexDB::IndexDB() {}

void IndexDB::Put(const std::string& key, EntryMeta* meta) {
  MutexLock lock(&mutex_);
  index_[key] = meta;
}

StatusCode IndexDB::Get(const std::string& key, EntryMeta** meta) {
  MutexLock lock(&mutex_);
  std::map<std::string, EntryMeta*>::iterator it = index_.find(key);
  if (it == index_.end()) {
    return kKeyNotFound;
  }
  *meta = index_[key];
  std::cerr << (*meta)->ToString() << std::endl;
  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
