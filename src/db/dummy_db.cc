// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include "dummy_db.h"

DummyDB::DummyDB() {
  pthread_mutex_init(&mutex_, NULL);
}

void DummyDB::Put(const std::string key, const std::string value, bool is_delete, int* status) {
  std::map<std::string, std::string>::iterator it;
  pthread_mutex_lock(&mutex_);
  if (is_delete) {
      it = db_.find(key);
      if (it == db_.end()) {
          *status = 1;
      } else {
          db_.erase(it);
      }
  } else {
      db_[key] = value;
  }
  for (std::map<std::string, std::string>::iterator tmp_it = db_.begin(); tmp_it != db_.end(); ++tmp_it) {
      //std::cout << tmp_it->first << "-" << tmp_it->second << std::endl;
  }
  pthread_mutex_unlock(&mutex_);
}

void DummyDB::Get(const std::string key, std::string* value, int* status) {
  pthread_mutex_lock(&mutex_);
  std::map<std::string, std::string>::iterator it = db_.find(key);
  if (it == db_.end()) {
      *status = 1;
  } else {
      *value = it->second;
  }
  pthread_mutex_unlock(&mutex_);
}
