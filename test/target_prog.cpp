#include <cstdint>
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

enum class MyEnum {
  CaseA,
  CaseB,
  CaseC,
};

enum class MyEnumInt8 : int8_t {
  CaseA,
  CaseB,
  CaseC,
};


enum MyUnscopedEnum {
  CASE_A,
  CASE_B,
  CASE_C,
};

typedef uint64_t UInt64;
using IntVector = std::vector<int>;

extern "C" {
void TestSimpleStruct(const SimpleStruct &x) {
}
void TestInheritance(const InheritanceChild &x) {
};
void TestContainer(const std::vector<SimpleStruct> &x) {
}
void TestEnum(MyEnum e) {
}
void TestEnumInt8(MyEnumInt8 e) {
}
void TestUnscopedEnum(MyUnscopedEnum e) {
}
void TestTypedef(UInt64 x) {
}
void TestUsing(IntVector &x) {
}
}

int main() {
  SimpleStruct ss;
  TestSimpleStruct(ss);

  InheritanceChild ic;
  TestInheritance(ic);

  std::vector<SimpleStruct> vss;
  TestContainer(vss);

  std::vector<int> vi;
  TestUsing(vi);

  return 0;
}
