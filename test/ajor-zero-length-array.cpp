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
  int arr1[1];
  int arr0[0];
  int arr2[2];
  int arr1x1[1][1];
  int arr1x2[1][2];
  int arr2x1[2][1];
  int arr2x0[2][0];
  int arr0x2[0][0];
  int end;
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
