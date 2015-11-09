// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace baidu {
namespace squirrel {

std::string EncodeDataEntry(const std::string& key, uint32_t key_len,
                            const std::string& value, uint32_t value_len) {
  boost::format fmter("%04d");
  fmter % key_len;
  std::string keylen_str = fmter.str();
  fmter % value_len;
  std::string valuelen_str = fmter.str();
  return keylen_str + valuelen_str + key + value;
}

void DecodeDataEntry(const std::string& entry, std::string* key, std::string* value) {
  uint32_t key_len = boost::lexical_cast<uint32_t>(entry.substr(0, 4));
  uint32_t value_len = boost::lexical_cast<uint32_t>(entry.substr(4, 4));
  if (key) {
    *key = entry.substr(8, key_len);
  }
  if (value) {
    *value = entry.substr(8 + key_len);
  }
}

} // namespace squirrel
} // namespace baidu
