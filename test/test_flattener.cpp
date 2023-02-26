// TODO
// empty parents

#include <gtest/gtest.h>

#include "type_graph/Flattener.h"
#include "type_graph/Printer.h"
#include "type_graph/Types.h"

using namespace type_graph;

// TODO delete after converting all tests to use Printer
void EXPECT_EQ_CLASS(const Class &actual, const Class &expected) {
//  EXPECT_EQ(actual.kind_, expected.kind_); TODO
  EXPECT_EQ(actual.name(), expected.name());
  EXPECT_EQ(actual.size(), expected.size());
  ASSERT_EQ(actual.members.size(), expected.members.size());
  for (std::size_t i=0; i<actual.members.size(); i++) {
    EXPECT_EQ(actual.members[i].type, expected.members[i].type);
    EXPECT_EQ(actual.members[i].name, expected.members[i].name);
    EXPECT_EQ(actual.members[i].offset, expected.members[i].offset);
  }
  ASSERT_EQ(actual.parents.size(), expected.parents.size());
  for (std::size_t i=0; i<actual.parents.size(); i++) {
    EXPECT_EQ(actual.parents[i].type, expected.parents[i].type);
    EXPECT_EQ(actual.parents[i].offset, expected.parents[i].offset);
  }
  ASSERT_EQ(actual.templateParams.size(), expected.templateParams.size());
  for (std::size_t i=0; i<actual.templateParams.size(); i++) {
    EXPECT_EQ(actual.templateParams[i].type, expected.templateParams[i].type);
    // TODO check template values
  }
}

void test(std::vector<std::reference_wrapper<Type>> types, std::string_view expected) {
  Flattener flattener;
  flattener.flatten(types);

  std::stringstream out;
  Printer printer(out);

  for (const auto &type : types) {
    printer.print(type);
  }

  // TODO standardise expected-actual order
  expected.remove_prefix(1); // Remove initial '\n'
  EXPECT_EQ(expected, out.str());
}

TEST(FlattenerTest, NoParents) {
  // Original and flattened:
  //   struct MyStruct { int n0; };
  //   class MyClass {
  //     int n;
  //     MyEnum e;
  //     MyStruct mystruct;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myenum = std::make_unique<Enum>("MyEnum", 4);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 4);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 12);

  mystruct->members.push_back(Member(myint.get(), "n0", 0));

  myclass->members.push_back(Member(myint.get(), "n", 0));
  myclass->members.push_back(Member(myenum.get(), "e", 4));
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 8));

  test({*myclass}, R"(
[0] Class: MyClass (size: 12)
      Member: n (offset: 0)
        Primitive: int32_t
      Member: e (offset: 4)
        Enum: MyEnum (size: 4)
      Member: mystruct (offset: 8)
[1]     Struct: MyStruct (size: 4)
          Member: n0 (offset: 0)
            Primitive: int32_t
)");
}

TEST(FlattenerTest, OnlyParents) {
  // Original:
  //   class C { int c; };
  //   class B { int b; };
  //   class A : B, C { };
  //
  // Flattened:
  //   class A {
  //     int b;
  //     int c;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));
  classB->members.push_back(Member(myint.get(), "b", 0));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->parents.push_back(Parent(classC.get(), 4));

  test({*classA}, R"(
[0] Class: ClassA (size: 8)
      Member: b (offset: 0)
        Primitive: int32_t
      Member: c (offset: 4)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, ParentsFirst) {
  // Original:
  //   class C { int c; };
  //   class B { int b; };
  //   class A : B, C { int a; };
  //
  // Flattened:
  //   class A {
  //     int b;
  //     int c;
  //     int a;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));
  classB->members.push_back(Member(myint.get(), "b", 0));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->parents.push_back(Parent(classC.get(), 4));
  classA->members.push_back(Member(myint.get(), "a", 8));

  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: b (offset: 0)
        Primitive: int32_t
      Member: c (offset: 4)
        Primitive: int32_t
      Member: a (offset: 8)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, MembersFirst) {
  // Original:
  //   class C { int c; };
  //   class B { int b; };
  //   class A : B, C { int a; };
  //
  // Flattened:
  //   class A {
  //     int a;
  //     int b;
  //     int c;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->members.push_back(Member(myint.get(), "b", 0));

  classA->members.push_back(Member(myint.get(), "a", 0));
  classA->parents.push_back(Parent(classB.get(), 4));
  classA->parents.push_back(Parent(classC.get(), 8));

  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: a (offset: 0)
        Primitive: int32_t
      Member: b (offset: 4)
        Primitive: int32_t
      Member: c (offset: 8)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, MixedMembersAndParents) {
  // Original:
  //   class C { int c; };
  //   class B { int b; };
  //   class A : B, C { int a1; int a2; int a3; };
  //
  // Flattened:
  //   class A {
  //     int a1;
  //     int b;
  //     int a2;
  //     int c;
  //     int a3;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 20);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->members.push_back(Member(myint.get(), "b", 0));

  classA->members.push_back(Member(myint.get(), "a1", 0));
  classA->parents.push_back(Parent(classB.get(), 4));
  classA->members.push_back(Member(myint.get(), "a2", 8));
  classA->parents.push_back(Parent(classC.get(), 12));
  classA->members.push_back(Member(myint.get(), "a3", 16));

  test({*classA}, R"(
[0] Class: ClassA (size: 20)
      Member: a1 (offset: 0)
        Primitive: int32_t
      Member: b (offset: 4)
        Primitive: int32_t
      Member: a2 (offset: 8)
        Primitive: int32_t
      Member: c (offset: 12)
        Primitive: int32_t
      Member: a3 (offset: 16)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, EmptyParent) {
  // Original:
  //   class C { int c; };
  //   class B { };
  //   class A : B, C { int a1; int a2; };
  //
  // Flattened:
  //   class A {
  //     int c;
  //     int a1;
  //     int a2;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 0);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classA->members.push_back(Member(myint.get(), "a1", 4));
  classA->members.push_back(Member(myint.get(), "a2", 8));
  classA->parents.push_back(Parent(classB.get(), 4));
  classA->parents.push_back(Parent(classC.get(), 0));

  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: c (offset: 0)
        Primitive: int32_t
      Member: a1 (offset: 4)
        Primitive: int32_t
      Member: a2 (offset: 8)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, TwoDeep) {
  // Original:
  //   class D { int d; };
  //   class C { int c; };
  //   class B : D { int b; };
  //   class A : B, C { int a; };
  //
  // Flattened:
  //   class A {
  //     int d;
  //     int b;
  //     int c;
  //     int a;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 16);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);
  auto classD = std::make_unique<Class>(Class::Kind::Class, "ClassD", 4);

  classD->members.push_back(Member(myint.get(), "d", 0));

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->parents.push_back(Parent(classD.get(), 0));
  classB->members.push_back(Member(myint.get(), "b", 4));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->parents.push_back(Parent(classC.get(), 8));
  classA->members.push_back(Member(myint.get(), "a", 12));

  test({*classA}, R"(
[0] Class: ClassA (size: 16)
      Member: d (offset: 0)
        Primitive: int32_t
      Member: b (offset: 4)
        Primitive: int32_t
      Member: c (offset: 8)
        Primitive: int32_t
      Member: a (offset: 12)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, DiamondInheritance) {
  // Original:
  //   class C { int c; };
  //   class B : C { int b; };
  //   class A : B, C { int a; };
  //
  // // TODO Decide on deduplication strategy
  // Flattened:
  //   class A {
  //     int c0;
  //     int b;
  //     int c1;
  //     int a;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 16);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->parents.push_back(Parent(classC.get(), 0));
  classB->members.push_back(Member(myint.get(), "b", 4));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->parents.push_back(Parent(classC.get(), 8));
  classA->members.push_back(Member(myint.get(), "a", 12));

  // TODO duplicate names for "c" member variables
  test({*classA}, R"(
[0] Class: ClassA (size: 16)
      Member: c (offset: 0)
        Primitive: int32_t
      Member: b (offset: 4)
        Primitive: int32_t
      Member: c (offset: 8)
        Primitive: int32_t
      Member: a (offset: 12)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, DuplicateMemberNames) {
  // Original:
  //   class C { int n; };
  //   class B { int n; };
  //   class A : B, C { int n; };
  //
  // // TODO Decide on deduplication strategy
  // Flattened:
  //   class A {
  //     int n0;
  //     int n1;
  //     int n2;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "n", 0));

  classB->members.push_back(Member(myint.get(), "n", 0));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->parents.push_back(Parent(classC.get(), 4));
  classA->members.push_back(Member(myint.get(), "n", 8));

  // TODO duplicate names for "c" member variables
  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: n (offset: 0)
        Primitive: int32_t
      Member: n (offset: 4)
        Primitive: int32_t
      Member: n (offset: 8)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, Member) {
  // Original:
  //   class C { int c; };
  //   class B : C { int b; };
  //   class A { int a; B b; };
  //
  // Flattened:
  //   class B { int c; int b; };
  //   Class A { int a; B b; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->parents.push_back(Parent(classC.get(), 0));
  classB->members.push_back(Member(myint.get(), "b", 4));

  classA->members.push_back(Member(myint.get(), "a", 0));
  classA->members.push_back(Member(classB.get(), "b", 4));

  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: a (offset: 0)
        Primitive: int32_t
      Member: b (offset: 4)
[1]     Class: ClassB (size: 8)
          Member: c (offset: 0)
            Primitive: int32_t
          Member: b (offset: 4)
            Primitive: int32_t
)");
}

TEST(FlattenerTest, MemberOfParent) {
  // Original:
  //   class C { int c; };
  //   class B { int b; C c; };
  //   class A : B { int a; };
  //
  // Flattened:
  //   class C { int c; };
  //   class A { int b; C c; int a; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->members.push_back(Member(myint.get(), "b", 0));
  classB->members.push_back(Member(classC.get(), "c", 4));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 8));

  test({*classA}, R"(
[0] Class: ClassA (size: 12)
      Member: b (offset: 0)
        Primitive: int32_t
      Member: c (offset: 4)
[1]     Class: ClassC (size: 4)
          Member: c (offset: 0)
            Primitive: int32_t
      Member: a (offset: 8)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, ContainerParam) {
  // Original:
  //   class B { int b; };
  //   class A : B { int a; };
  //   std::vector<A, int>
  //
  // Flattened:
  //   class A { int b; int a; };
  //   std::vector<A, int>
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  auto container = std::make_unique<Container>(SEQ_TYPE);

  classB->members.push_back(Member(myint.get(), "b", 0));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));

  container->templateParams.push_back(TemplateParam(classA.get()));
  container->templateParams.push_back(TemplateParam(myint.get()));

  test({*container}, R"(
[0] Container: std::vector
      Param
[1]     Class: ClassA (size: 8)
          Member: b (offset: 0)
            Primitive: int32_t
          Member: a (offset: 4)
            Primitive: int32_t
      Param
        Primitive: int32_t
)");
}

TEST(FlattenerTest, Array) {
  // Original:
  //   class B { int b; };
  //   class A : B { int a; };
  //   A[5]
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  classB->members.push_back(Member(myint.get(), "b", 0));

  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));

  auto arrayA = std::make_unique<Array>("TODO remove this string", 5, classA.get());

  test({*arrayA}, R"(
[0] Array: (length: 5)
[1]   Class: ClassA (size: 8)
        Member: b (offset: 0)
          Primitive: int32_t
        Member: a (offset: 4)
          Primitive: int32_t
)");
}

TEST(FlattenerTest, Typedef) {
  // Original:
  //   class B { int b; };
  //   class A : B { int a; };
  //   using aliasA = A;
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  classB->members.push_back(Member(myint.get(), "b", 0));

  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));

  auto aliasA = std::make_unique<Typedef>("aliasA", classA.get());

  test({*aliasA}, R"(
[0] Typedef: aliasA
[1]   Class: ClassA (size: 8)
        Member: b (offset: 0)
          Primitive: int32_t
        Member: a (offset: 4)
          Primitive: int32_t
)");
}

TEST(FlattenerTest, TypedefParent) {
  // Original:
  //   class B { int b; };
  //   using aliasB = B;
  //   class A : aliasB { int a; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  classB->members.push_back(Member(myint.get(), "b", 0));

  auto aliasB = std::make_unique<Typedef>("aliasB", classB.get());

  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  classA->parents.push_back(Parent(aliasB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));

  test({*classA}, R"(
[0] Class: ClassA (size: 8)
      Member: b (offset: 0)
        Primitive: int32_t
      Member: a (offset: 4)
        Primitive: int32_t
)");
}

TEST(FlattenerTest, Pointer) {
  // Original:
  //   class B { int b; };
  //   class A : B { int a; };
  //   class C { A *a; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 4);
  classB->members.push_back(Member(myint.get(), "b", 0));

  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));

  auto ptrA = std::make_unique<Pointer>(classA.get());
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 8);
  classC->members.push_back(Member(ptrA.get(), "a", 0));

  test({*classC}, R"(
[0] Class: ClassC (size: 8)
      Member: a (offset: 0)
[1]     Pointer
[2]       Class: ClassA (size: 8)
            Member: b (offset: 0)
              Primitive: int32_t
            Member: a (offset: 4)
              Primitive: int32_t
)");
}

TEST(FlattenerTest, PointerCycle) {
  // Original:
  //   class B { A* a };
  //   class A { B b; };
  //
  // Flattened:
  //   No change
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
  auto ptrA = std::make_unique<Pointer>(classA.get());
  classA->members.push_back(Member(classB.get(), "b", 0));
  classB->members.push_back(Member(ptrA.get(), "a", 0));

  test({*classA, *classB}, R"(
[0] Class: ClassA (size: 69)
      Member: b (offset: 0)
[1]     Class: ClassB (size: 69)
          Member: a (offset: 0)
[2]         Pointer
              [0]
    [1]
)");
}
