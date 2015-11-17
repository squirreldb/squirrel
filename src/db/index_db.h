// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_INDEX_DB_H_
#define SQUIRREL_INDEX_DB_H_

#include <string>

#include <mutex.h>
#include <boost/lexical_cast.hpp>

#include "src/proto/status_code.pb.h"

namespace baidu {
namespace squirrel {
namespace db {

struct EntryMeta
{
  std::string filename;
  uint32_t offset;
  uint32_t length;

  std::string ToString() {
    return "filename=" + filename +
           " offset=" + boost::lexical_cast<std::string>(offset) +
           " length=" + boost::lexical_cast<std::string>(length);
    }
};

class IndexDB {
public:
  virtual ~IndexDB() {}
  virtual StatusCode Put(const std::string& key, EntryMeta* meta) = 0;
  virtual StatusCode Get(const std::string& key, EntryMeta* meta) = 0;
  virtual StatusCode Delete(const std::string& key) = 0;
};

} // namespace db
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_INDEX_DB_H_
