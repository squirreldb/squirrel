// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

namespace baidu {
namespace squirrel {

void EncodeFixed32(char* buf, uint64_t value) {
  buf[0] = value & 0xff;
  buf[1] = (value >> 8) & 0xff;
  buf[2] = (value >> 16) & 0xff;
  buf[3] = (value >> 24) & 0xff;
}

void EncodeDataEntry(const std::string& key, uint32_t key_len,
                     const std::string& value, uint32_t value_len, char* dst) {
  char* p = dst;
  EncodeFixed32(p, key_len);
  p +=4;
  EncodeFixed32(p, value_len);
  p +=4;
  memcpy(p, key.c_str(), key_len);
  p += key_len;
  memcpy(p, value.c_str(), value_len);
  assert(static_cast<size_t>((p + value_len) - dst) == 8 + key_len + value_len);
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

void GetDataFilename(uint64_t* file_num, std::string* filename) {
  *file_num += 1;
  filename->clear();
  filename->assign(boost::lexical_cast<std::string>(*file_num));
  filename->append(".data");
}

} // namespace squirrel
} // namespace baidu
