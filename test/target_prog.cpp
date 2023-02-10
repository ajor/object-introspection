#include <cstdint>
#include <vector>

class SimpleClass {
  int a;
  char b;
  long long c;
};

struct SimpleStruct {
  int a;
  char b;
  long long c;
};

union SimpleUnion {
  int a;
  char b;
  long long c;
};

class InheritanceBase {
  int a;
};
class InheritanceMiddle : InheritanceBase {
  int b;
};
class InheritanceChild : InheritanceMiddle {
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

struct CyclicalNode {
  struct CyclicalNode *next;
  int val;
};

struct ArrayStruct {
  int int_array[5];
  SimpleStruct struct_array[3];
};

extern "C" {
void TestSimpleClass(const SimpleClass &x) {
}
void TestSimpleStruct(const SimpleStruct &x) {
}
void TestSimpleUnion(const SimpleUnion &x) {
}
void TestInheritance(const InheritanceChild &x) {
};
void TestContainer(const std::vector<SimpleStruct> &x) {
}
void TestEnum(MyEnum x) {
}
void TestEnumInt8(MyEnumInt8 x) {
}
void TestUnscopedEnum(MyUnscopedEnum x) {
}
void TestTypedef(UInt64 x) {
}
void TestUsing(const IntVector &x) {
}
void TestCycle(const CyclicalNode &x) {
}
void TestArray(const ArrayStruct &x) {
}
}

int main() {
  SimpleClass sc;
  TestSimpleClass(sc);

  SimpleStruct ss;
  TestSimpleStruct(ss);

  SimpleUnion su;
  TestSimpleUnion(su);

  InheritanceChild ic;
  TestInheritance(ic);

  std::vector<SimpleStruct> vss;
  TestContainer(vss);

  std::vector<int> vi;
  TestUsing(vi);

  CyclicalNode cn;
  TestCycle(cn);

  ArrayStruct as;
  TestArray(as);

  return 0;
}
