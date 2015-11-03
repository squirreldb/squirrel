// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include "db.h"

namespace baidu {
namespace squirrel {
namespace db {

DB::DB() {
  fmter_ = new boost::format("%04d");
}

void DB::Put(const std::string& key, const std::string& value, int* status) {
  MutexLock lock(&mutex_);

}

void DB::Get(const std::string& key, std::string* value, int* status) {
  MutexLock lock(&mutex_);
}

} // namespace db
} // namespace squirrel
} // namespace baidu
