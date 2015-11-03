// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include <thread.h>
#include <boost/bind.hpp>

#include "src/client/client.h"

void test_put(baidu::squirrel::sdk::Client* client, std::string& key,
              std::string& value) {
  while (true) {
    int thread_pool_pending;
    client->GetStat(NULL, NULL, NULL, &thread_pool_pending, NULL);
    while (thread_pool_pending > 100) {
      usleep(500);
      client->GetStat(NULL, NULL, NULL, &thread_pool_pending, NULL);
    }
    client->Put(key, value);
  }
}

int main() {
  struct timeval tv_start, tv_end;
  gettimeofday(&tv_start, NULL);

  baidu::squirrel::sdk::Client client;
  std::string key = "k";
  std::string value = "v";

  baidu::common::Thread thread;
  thread.Start(boost::bind(&test_put, &client, key, value));

  while (true) {
    gettimeofday(&tv_end, NULL);
    long start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
    long end = tv_end.tv_sec * 1000000 + tv_end.tv_usec;
    double interval = (end - start) / double(1000000);

    int count, failed, pending, thread_pool_pending;
    std::string profile_str;
    client.GetStat(&count, &failed, &pending, &thread_pool_pending, &profile_str);
    std::cout << "Qps=" << int(count / interval)
              << "\tfailed=" << int(failed / interval)
              << "\tpending=" << int(pending / interval)
              << "\tinterval=" << interval
              << "\tthreadpool_pending=" << thread_pool_pending
              << "\ttp=" << profile_str
              << std::endl;
    client.ResetStat();

    tv_start = tv_end;
    sleep(1);
  }

  thread.Join();
  return EXIT_SUCCESS;
}
