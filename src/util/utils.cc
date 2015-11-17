// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

namespace baidu {
namespace squirrel {

void EncodeDataEntry(const std::string& key, const std::string& value,
                     uint32_t encode_lengtch, char* dst) {
  uint32_t key_len = key.length();
  uint32_t value_len = value.length();
  char* p = dst;
  memcpy(p, &key_len, sizeof(key_len));
  p +=4;
  memcpy(p, &value_len, sizeof(value_len));
  p +=4;
  memcpy(p, key.c_str(), key_len);
  p += key_len;
  memcpy(p, value.c_str(), value_len);
  assert(static_cast<size_t>((p + value_len) - dst) == encode_lengtch);
}

void DecodeDataEntry(const char* entry, std::string* key, std::string* value) {
  uint32_t key_len;
  memcpy(&key_len, entry, sizeof(key_len));
  uint32_t value_len;
  memcpy(&value_len, entry + 4, sizeof(value_len));
  if (key) {
    key->assign(entry + 8, key_len);
  }
  if (value) {
    value->assign(entry + 8 + key_len, value_len);
  }
}

void EncodeIndexEntry(uint32_t offset, uint32_t length, const std::string& filename,
                      std::string* res) {
  char buf[8];
  memcpy(buf, &offset, sizeof(offset));
  memcpy(buf + 4, &length, sizeof(length));
  *res = filename;
  res->append(buf, 8);
}

void DecodeIndexEntry(const std::string& input, uint32_t* offset, uint32_t* length,
                      std::string* filename) {
  size_t input_size = input.size();
  memcpy(offset, &input[input_size - 8], sizeof(uint32_t));
  memcpy(length, &input[input_size - 4], sizeof(uint32_t));
  filename->assign(input, 0, input_size - 8);
}

void GetDataFilename(uint32_t* file_num, std::string* filename) {
  *file_num += 1;
  filename->clear();
  filename->assign(boost::lexical_cast<std::string>(*file_num));
  filename->append(".data");
}

} // namespace squirrel
} // namespace baidu
