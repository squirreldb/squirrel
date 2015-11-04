// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_INDEX_DB_H_
#define SQUIRREL_INDEX_DB_H_

#include <map>
#include <string>

#include <mutex.h>

namespace baidu {
namespace squirrel {
namespace db {

struct EntryMeta
{
  std::string filename;
  int64_t offset;
  int32_t length;
};

class IndexDB {
public:
  IndexDB();
  void Put(const std::string& key, EntryMeta* meta);

private:
  Mutex mutex_;
  std::map<std::string, EntryMeta*> index_;
};

} // namespace db
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_INDEX_DB_H_
