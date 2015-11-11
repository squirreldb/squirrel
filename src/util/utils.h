// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_UTIL_UTILS_H_
#define SQUIRREL_UTIL_UTILS_H_

namespace baidu {
namespace squirrel {

void EncodeFixed32(char* buf, uint32_t value);
void EncodeDataEntry(const std::string& key, uint32_t key_len,
                     const std::string& value, uint32_t value_len, char* dst);
void DecodeDataEntry(const char* entry, std::string* key, std::string* value);
void GetDataFilename(uint64_t* file_num, std::string* filename);
} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_UTIL_UTILS_H_
