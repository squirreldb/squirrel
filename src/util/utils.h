// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SQUIRREL_UTIL_UTILS_H_
#define SQUIRREL_UTIL_UTILS_H_

namespace baidu {
namespace squirrel {

void EncodeDataEntry(const std::string& key, const std::string& value,
                     uint32_t encode_lengtch, char* dst);
void DecodeDataEntry(const char* entry, std::string* key, std::string* value);
void EncodeIndexEntry(uint32_t offset, uint32_t length, const std::string& filename,
                      std::string* res);
void DecodeIndexEntry(const std::string& input, uint32_t* offset, uint32_t* length,
                      std::string* filename);
void GetDataFilename(uint32_t* file_num, std::string* filename);

} // namespace squirrel
} // namespace baidu

#endif // SQUIRREL_UTIL_UTILS_H_
