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

class Foo {
 public:
  int ref;
  int base[64];
  int __base0;
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
