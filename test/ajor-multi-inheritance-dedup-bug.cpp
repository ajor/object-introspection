#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <chrono>
#include <future>
#include <iostream>
#include <map>
#include <vector>

class BaseFoo {
  int base[64];
};

class Base2 : BaseFoo {
  int base2_mem;
};

class Base3 : BaseFoo {
  int base3_mem;
};

class Foo : BaseFoo, Base2, Base3 {
 public:
  int ref;
};

std::vector<int> doStuff(Foo &foo,
                         std::vector<std::map<std::string, std::string>> &m,
                         std::vector<std::string> &f,
                         std::vector<std::pair<std::string, double>> &p) {
  foo.ref++;

  return {};
}

int main(int argc, char *argv[]) {
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  exit(EXIT_SUCCESS);
}
