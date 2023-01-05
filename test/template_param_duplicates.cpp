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

namespace NS1 {
	struct Bar {
		int i;
	};
}

namespace NS2 {
	struct Bar {
		double d;
	};
}

template <typename T>
class Bar {
  T x;
};

class Foo {
  public:
int ref;
std::vector<NS1::Bar> v1;
std::vector<NS2::Bar> v2;
NS1::Bar bar1;
NS2::Bar bar2;
::Bar<NS1::Bar> w1;
::Bar<NS2::Bar> w2;
std::vector<int> vvv;
};

std::vector<int> doStuff(Foo &foo,
                         std::vector<std::map<std::string, std::string>> &m,
                         std::vector<std::string> &f,
                         std::vector<std::pair<std::string, double>> &p) {
  foo.ref++;
  return {};
}

int main(int argc, char *argv[]) {
  std::this_thread::sleep_for(std::chrono::milliseconds(10000000));
  exit(EXIT_SUCCESS);
}
