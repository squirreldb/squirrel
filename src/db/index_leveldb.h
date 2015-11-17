// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_INDEX_LEVEL_DB_H_
#define SQUIRREL_INDEX_LEVEL_DB_H_

#include <leveldb/db.h>

#include "index_db.h"

namespace baidu {
namespace squirrel {
namespace db {

class IndexLevelDB : public IndexDB {
public:
  IndexLevelDB(const std::string& dbname);
  virtual ~IndexLevelDB() {};
  virtual StatusCode Put(const std::string& key, EntryMeta* meta);
  virtual StatusCode Get(const std::string& key, EntryMeta* meta);
  virtual StatusCode Delete(const std::string& key);

private:
  leveldb::DB* index_;
};

} // namespace db
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_INDEX_LEVEL_DB_H_