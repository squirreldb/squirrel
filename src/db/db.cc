// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "db.h"
#include "src/util/utils.h"

namespace baidu {
namespace squirrel {
namespace db {

DB::DB() : file_num_(0), offset_(0), index_(new IndexDB()) {
  filename_ = boost::lexical_cast<std::string>(file_num_);
  filename_ += ".data";
  std::cerr << "write to file " << filename_ << std::endl;
  fout_ = open(filename_.c_str(), O_WRONLY | O_CREAT);
}

StatusCode DB::Put(const std::string& key, const std::string& value) {
  uint32_t key_len = key.length();
  uint32_t value_len = value.length();
  EntryMeta* meta = new EntryMeta();

  MutexLock lock(&mutex_);
  std::string entry = EncodeDataEntry(key, key_len, value, value_len);
  write(fout_, entry.c_str(), entry.size());
  meta->offset = offset_;
  meta->length = 8 + key_len + value_len;
  meta->filename = filename_;
  offset_ += meta->length;
  std::cerr << meta->ToString() << std::endl;
  mutex_.Unlock();

  return index_->Put(key, meta);
}

StatusCode DB::Get(const std::string& key, std::string* value) {
  EntryMeta* meta;
  StatusCode index_status = index_->Get(key, &meta);
  if (index_status != kOK) {
    return index_status;
  }
  std::cerr << "Got index " << meta->ToString() << std::endl;
  int fp = open(meta->filename.c_str(), O_RDONLY);
  lseek(fp, meta->offset, SEEK_SET);
  char data[meta->length];
  read(fp, data, meta->length);
  std::cerr << "read:" << data << std::endl;
  std::string entry(data, meta->length);
  DecodeDataEntry(entry, NULL, value);
  std::cerr << "value:" << *value << std::endl;
  return kOK;
}

StatusCode DB::Delete(const std::string& key) {
  StatusCode index_status = index_->Delete(key);
  return index_status;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
