#include <cstdint>
#include <vector>

typedef uint64_t UInt64;
using IntVector = std::vector<int>;

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

class ClassFunctions {
  int memberA;
  void foo();
  int bar(char *s);
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
void TestContainerAllocator(const std::vector<SimpleStruct> &x) {
}
void TestTypedef(UInt64 x) {
}
void TestUsing(const IntVector &x) {
}
void TestArray(const ArrayStruct &x) {
}
void TestClassTemplateInt(const TemplatedClass1<int> &x) {
}
void TestClassTemplateVector(const TemplatedClass1<std::vector<int>> &x) {
}
void TestClassTemplateSimpleStructInt(const TemplatedClass2<SimpleStruct, int> &x) {
}
void TestClassFunctions(const ClassFunctions &x) {
}
}

int main() {
  std::vector<SimpleStruct> vss;
  TestContainer(vss);

  std::vector<int> vi;
  TestUsing(vi);

  ArrayStruct as;
  TestArray(as);

  TemplatedClass1<int> tc1int;
  TestClassTemplateInt(tc1int);

  TemplatedClass1<std::vector<int>> tc1vec;
  TestClassTemplateVector(tc1vec);

  TemplatedClass2<SimpleStruct, int> tc2ssint;
  TestClassTemplateSimpleStructInt(tc2ssint);

  ClassFunctions cf;
  TestClassFunctions(cf);

  return 0;
}
