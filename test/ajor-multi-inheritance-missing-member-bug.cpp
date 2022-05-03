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

struct ClassBase {
  int mem_a;
};

struct ClassInt : public ClassBase {
};

struct ClassTop : public ClassInt {
};

class Foo {
 public:
  int ref;
  ClassTop cc;
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
