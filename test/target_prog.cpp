#include <vector>

struct SimpleStruct {
  int a;
  int b;
  int c;
};
extern "C" {
void TestSimpleStruct(const SimpleStruct &x) {
}
}

struct InheritanceBase {
  int a;
};
struct InheritanceMiddle : InheritanceBase {
  int b;
};
struct InheritanceChild : InheritanceMiddle {
  int c;
};
void TestInheritance(const InheritanceChild &x) {
};

void TestContainer(const std::vector<SimpleStruct> &x) {
}

int main() {
  return 0;
}
