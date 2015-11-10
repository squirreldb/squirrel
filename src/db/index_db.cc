// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "index_db.h"

namespace baidu {
namespace squirrel {
namespace db {

IndexDB::IndexDB() {}

StatusCode IndexDB::Put(const std::string& key, EntryMeta* meta) {
  MutexLock lock(&mutex_);
  std::cerr << "put: " << meta->ToString() << std::endl;
  index_[key] = meta;
  return kOK;
}

StatusCode IndexDB::Get(const std::string& key, EntryMeta** meta) {
  MutexLock lock(&mutex_);
  std::map<std::string, EntryMeta*>::iterator it = index_.find(key);
  if (it == index_.end()) {
    return kKeyNotFound;
  }
  *meta = it->second;
  std::cerr << "get: " << (*meta)->ToString() << std::endl;
  return kOK;
}

StatusCode IndexDB::Delete(const std::string& key) {
  MutexLock lock(&mutex_);
  std::map<std::string, EntryMeta*>::iterator it = index_.find(key);
  if (it == index_.end()) {
    return kKeyNotFound;
  }
  EntryMeta* meta = it->second;
  std::cerr << "delete: " << meta->ToString() << std::endl;
  index_.erase(it);
  delete meta;
  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
