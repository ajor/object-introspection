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
  int intArray[5];
  SimpleStruct structArray[3];
  char charArray[0];
};

template <typename T>
class TemplatedClass1 {
  T templatedValue;
};

template <typename T, typename S>
class TemplatedClass2 {
  TemplatedClass1<T> tc1;
  S arr[2];
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
void TestClassTemplateInt(const TemplatedClass1<int> &x) {
}
void TestClassTemplateVector(const TemplatedClass1<std::vector<int>> &x) {
}
void TestClassTemplateSimpleStructInt(const TemplatedClass2<SimpleStruct, int> &x) {
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

  TemplatedClass1<int> tc1int;
  TestClassTemplateInt(tc1int);

  TemplatedClass1<std::vector<int>> tc1vec;
  TestClassTemplateVector(tc1vec);

  TemplatedClass2<SimpleStruct, int> tc2ssint;
  TestClassTemplateSimpleStructInt(tc2ssint);

  return 0;
}
