// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "db.h"

namespace baidu {
namespace squirrel {
namespace db {

DB::DB() : file_num_(0), offset_(0) {
  filename_ = boost::lexical_cast<std::string>(file_num_);
  filename_ += ".data";
  std::cerr << "write to file " << filename_ << std::endl;
  fout_ = new std::ofstream(filename_.c_str(), std::ofstream::out);
}

void DB::Put(const std::string& key, uint32_t key_len,
                 const std::string& value, uint32_t value_len,
                 uint32_t* offset, std::string* filename, int* status) {
  boost::format fmter("%04d");
  fmter % key_len;
  std::string keylen_str = fmter.str();
  fmter % value_len;
  std::string valuelen_str = fmter.str();

  MutexLock lock(&mutex_);
  (*fout_) << keylen_str << key << valuelen_str << value << std::endl;
  *offset = offset_;
  *filename = filename_;
  offset_ += 4 + key_len + 4 + value_len;
}

void DB::Get(const std::string& key, std::string* value, int* status) {
  MutexLock lock(&mutex_);
}

} // namespace db
} // namespace squirrel
} // namespace baidu
