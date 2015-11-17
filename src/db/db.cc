// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "db.h"
#include "index_leveldb.h"
#include "src/util/utils.h"

extern int CONF_db_file_size;

namespace baidu {
namespace squirrel {
namespace db {

DB::DB() : file_num_(0), offset_(0), file_size_limit_(CONF_db_file_size << 20),
           index_(new IndexLevelDB("index_db")) {
  GetDataFilename(&file_num_, &filename_);
  std::cerr << "write to file " << filename_ << std::endl;
  fout_ = open(filename_.c_str(), O_WRONLY | O_CREAT,
               S_IREAD | S_IWRITE | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
}

StatusCode DB::Put(const std::string& key, const std::string& value) {
  uint32_t key_len = key.length();
  uint32_t value_len = value.length();
  EntryMeta meta;

  MutexLock lock(&mutex_);
  uint32_t data_size = 8 + key_len + value_len;
  char buf[data_size];
  EncodeDataEntry(key, key_len, value, value_len, buf);
  write(fout_, buf, data_size);

  meta.offset = offset_;
  meta.length = 8 + key_len + value_len;
  meta.filename = filename_;
  offset_ += meta.length;
  //std::cerr << meta.ToString() << std::endl;
  mutex_.Unlock();

  SwitchFile();

  return index_->Put(key, &meta);
}

StatusCode DB::Get(const std::string& key, std::string* value) {
  EntryMeta meta;
  StatusCode index_status = index_->Get(key, &meta);
  if (index_status != kOK) {
    return index_status;
  }
  std::cerr << "Got index " << meta.ToString() << std::endl;
  int fp = open(meta.filename.c_str(), O_RDONLY);
  if (fp < 0) {
    return kIOError;
  }

  lseek(fp, meta.offset, SEEK_SET);
  char entry[meta.length];
  read(fp, entry, meta.length);
  DecodeDataEntry(entry, NULL, value);
  std::cerr << "value:" << *value << std::endl;
  return kOK;
}

StatusCode DB::Delete(const std::string& key) {
  StatusCode index_status = index_->Delete(key);
  return index_status;
}

StatusCode DB::Scan(const std::string& start, const std::string& end, KvPairResults* results,
                    bool* complete) {

}

StatusCode DB::SwitchFile() {
  MutexLock lock(&mutex_);
  if (offset_ >= file_size_limit_) {
    int ret = close(fout_);
    if (ret != 0) {
      return kIOError;
    }
    GetDataFilename(&file_num_, &filename_);
    fout_ = open(filename_.c_str(), O_WRONLY | O_CREAT,
                 S_IREAD | S_IWRITE | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fout_ < 0) {
      return kIOError;
    }
    offset_ = 0;
    std::cerr << "switch to new data file " << filename_ << std::endl;
  }
  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
