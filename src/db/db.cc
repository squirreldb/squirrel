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

DB::DB() : file_num_(0), offset_(0), index_(new IndexDB()) {
  filename_ = boost::lexical_cast<std::string>(file_num_);
  filename_ += ".data";
  std::cerr << "write to file " << filename_ << std::endl;
  fout_ = new std::ofstream(filename_.c_str(), std::ofstream::out);
}

void DB::Put(const std::string& key, const std::string& value, int* status) {
  uint32_t key_len = key.length();
  uint32_t value_len = value.length();
  boost::format fmter("%04d");
  fmter % key_len;
  std::string keylen_str = fmter.str();
  fmter % value_len;
  std::string valuelen_str = fmter.str();

  EntryMeta* meta = new EntryMeta();
  MutexLock lock(&mutex_);
  (*fout_) << keylen_str << valuelen_str << key << value << std::endl;
  meta->offset = offset_;
  meta->length = 4 + key_len + 4 + value_len;
  meta->filename = filename_;
  offset_ += meta->length;
  mutex_.Unlock();

  index_->Put(key, meta);
}

void DB::Get(const std::string& key, std::string* value, int* status) {
  MutexLock lock(&mutex_);
}

} // namespace db
} // namespace squirrel
} // namespace baidu
