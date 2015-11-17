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
  Recover();
  GetDataFilename(&file_num_, &filename_);
  std::cerr << "write to file " << filename_ << std::endl;
  fout_ = open(filename_.c_str(), O_WRONLY | O_CREAT,
               S_IREAD | S_IWRITE | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  LogFileNum();

}

StatusCode DB::Put(const std::string& key, const std::string& value) {
  uint32_t key_len = key.length();
  uint32_t value_len = value.length();

  uint32_t data_size = 8 + key_len + value_len;
  char buf[data_size];
  EncodeDataEntry(key, key_len, value, value_len, buf);
  MutexLock lock(&mutex_);
  write(fout_, buf, data_size);

  uint32_t offset = offset_;
  uint32_t length = 8 + key_len + value_len;
  std::string filename = filename_;
  offset_ += length;
  mutex_.Unlock();

  std::string index_value;
  EncodeIndexEntry(offset, length, filename, &index_value);
  // std::cerr << "Write index " << filename << ":" << offset << ":" << length << std::endl;
  StatusCode status = index_->Put(key, index_value);

  SwitchFile();
  return status;
}

StatusCode DB::Get(const std::string& key, std::string* value) {
  std::string index_value;
  StatusCode index_status = index_->Get(key, &index_value);
  if (index_status != kOK) {
    return index_status;
  }
  uint32_t offset, length;
  std::string filename;
  DecodeIndexEntry(index_value, &offset, &length, &filename);

  // std::cerr << "Got index " << filename << ":" << offset << ":" << length << std::endl;
  int fp = open(filename.c_str(), O_RDONLY);
  if (fp < 0) {
    return kIOError;
  }

  lseek(fp, offset, SEEK_SET);
  char entry[length];
  read(fp, entry, length);
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

void DB::Recover() {
  std::string value;
  StatusCode status = index_->Get("LastFileNum", &value);
  if (status != kKeyNotFound) {
    memcpy(&file_num_, value.c_str(), sizeof(file_num_));
    std::cerr << "LastFileNum " << value << ":" <<  file_num_ << std::endl;
  }
}

void DB::LogFileNum() {
  char buf[4];
  memcpy(buf, &file_num_, sizeof(file_num_));
  index_->Put("LastFileNum", std::string(buf, 4));
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
    LogFileNum();
  }
  return kOK;
}

} // namespace db
} // namespace squirrel
} // namespace baidu
