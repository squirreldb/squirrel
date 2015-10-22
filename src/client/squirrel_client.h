// Copyright (c) 2015, squirreldb. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_SQUIRREL_CLIENT_H
#define CLIENT_SQUIRREL_CLIENT_H

class SquirrelClient
{
public:
  SquirrelClient();
  ~SquirrelClient();
  void init();
  void Put(const std::string& key, const std::string& value, const bool is_delete);
  void DoPut(const std::string& key, const std::string& value, const bool is_delete);
  void Get(std::string key);
  void GetStat(int* count, int* failed, int* pending);
  void ResetStat();

private:
  void PutCallback(sofa::pbrpc::RpcController* cntl, Squirrel::PutRequest* request,
                   Squirrel::PutResponse* response);
  void GetCallback(sofa::pbrpc::RpcController* cntl, Squirrel::GetRequest* request,
                   Squirrel::GetResponse* response);

private:
  int count_;
  int failed_;
  int thread_num_;
  int pending_;
  pthread_mutex_t mutex_;
  ThreadPool* thread_pool_;

  sofa::pbrpc::RpcClient* rpc_client_;
  sofa::pbrpc::RpcChannel* rpc_channel_;
  Squirrel::SquirrelServer_Stub* stub_;
};

#endif // CLIENT_SQUIRREL_CLIENT_H
