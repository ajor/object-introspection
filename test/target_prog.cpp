#include <vector>

struct SimpleStruct {
  int a;
  int b;
  int c;
};

struct InheritanceBase {
  int a;
};
struct InheritanceMiddle : InheritanceBase {
  int b;
};
struct InheritanceChild : InheritanceMiddle {
  int c;
};

extern "C" {
void TestSimpleStruct(const SimpleStruct &x) {
}
void TestInheritance(const InheritanceChild &x) {
};
void TestContainer(const std::vector<SimpleStruct> &x) {
}
}

int main() {
  SimpleStruct ss;
  TestSimpleStruct(ss);

  InheritanceChild ic;
  TestInheritance(ic);

  std::vector<SimpleStruct> vss;
  TestContainer(vss);

  return 0;
}
