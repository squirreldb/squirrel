#include <pthread.h>
#include <iostream>
#include <string>
#include <map>

class DummyDB {
public:
    DummyDB();
    // status = 0: success
    // status = 1: not found
    // status = 2: other
    void Put(const std::string key, const std::string value, bool is_delete, int* status);
    void Get(const std::string key, std::string* value, int* status);

private:
    std::map<std::string, std::string> db_;
    pthread_mutex_t mutex_;
};
