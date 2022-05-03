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
  int base_val;
};

class Base2 {
  int base2_val;
};

class Base3 : Base2 {
  int base3_val;
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
