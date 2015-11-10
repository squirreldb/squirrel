// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

namespace baidu {
namespace squirrel {

std::string EncodeDataEntry(const std::string& key, uint32_t key_len,
                            const std::string& value, uint32_t value_len) {
  char size[8];
  sprintf(size, "%04d", key_len);
  sprintf(size + 4, "%04d", value_len);
  std::string res(size, 8);
  return res + key + value;
}

void DecodeDataEntry(const std::string& entry, std::string* key, std::string* value) {
  uint32_t key_len = boost::lexical_cast<uint32_t>(entry.substr(0, 4));
  uint32_t value_len = boost::lexical_cast<uint32_t>(entry.substr(4, 4));
  if (key) {
    *key = entry.substr(8, key_len);
  }
  if (value) {
    *value = entry.substr(8 + key_len, value_len);
  }
}

void GetDataFilename(uint64_t* file_num, std::string* filename) {
  *file_num += 1;
  filename->clear();
  filename->assign(boost::lexical_cast<std::string>(*file_num));
  filename->append(".data");
}

} // namespace squirrel
} // namespace baidu
