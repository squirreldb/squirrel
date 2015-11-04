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
  std::cerr << "index:" << key << ":" << meta->filename << "-"
            << meta->offset << "-" << meta->length << std::endl;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
