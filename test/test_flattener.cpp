// TODO
// empty parents

#include <gtest/gtest.h>

#include "type_graph/Flattener.h"
#include "type_graph/Types.h"

using namespace type_graph;

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
  auto mybool = std::make_unique<Primitive>(Primitive::Kind::Bool); // TODO unused variable
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 12);

  mystruct->members.push_back(Member(myint.get(), "n0", 0));

  myclass->members.push_back(Member(myint.get(), "n", 0));
  myclass->members.push_back(Member(myenum.get(), "e", 4));
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 8));

  auto expected = *myclass;

  Flattener flattener;
  flattener.flatten({myclass.get()});

  EXPECT_EQ_CLASS(*myclass, expected);
}

TEST(TypeFlattenerTest, OnlyParents) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);
  expected->members.push_back(Member(myint.get(), "b", 0));
  expected->members.push_back(Member(myint.get(), "c", 4));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, ParentsFirst) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  expected->members.push_back(Member(myint.get(), "b", 0));
  expected->members.push_back(Member(myint.get(), "c", 4));
  expected->members.push_back(Member(myint.get(), "a", 8));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, MembersFirst) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  expected->members.push_back(Member(myint.get(), "a", 0));
  expected->members.push_back(Member(myint.get(), "b", 4));
  expected->members.push_back(Member(myint.get(), "c", 8));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, MixedMembersAndParents) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 20);
  expected->members.push_back(Member(myint.get(), "a1", 0));
  expected->members.push_back(Member(myint.get(),  "b", 4));
  expected->members.push_back(Member(myint.get(), "a2", 8));
  expected->members.push_back(Member(myint.get(),  "c", 12));
  expected->members.push_back(Member(myint.get(), "a3", 16));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, EmptyParent) {
  // Original:
  //   class C { int c; };
  //   class B { };
  //   class A : B, C { int a1; int a2; };
  //
  // Flattened:
  //   class A {
  //     int a1;
  //     int a2;
  //     int c;
  //   };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 0);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classA->members.push_back(Member(myint.get(), "a1", 0));
  classA->members.push_back(Member(myint.get(), "a2", 4));
  classA->parents.push_back(Parent(classB.get(), 4));
  classA->parents.push_back(Parent(classC.get(), 8));

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  expected->members.push_back(Member(myint.get(), "a1", 0));
  expected->members.push_back(Member(myint.get(), "a2", 4));
  expected->members.push_back(Member(myint.get(),  "c", 8));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, TwoDeep) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 16);
  expected->members.push_back(Member(myint.get(), "d", 0));
  expected->members.push_back(Member(myint.get(), "b", 4));
  expected->members.push_back(Member(myint.get(), "c", 8));
  expected->members.push_back(Member(myint.get(), "a", 12));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, DiamondInheritance) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 16);
  // TODO duplicate names for "c" member variables
  expected->members.push_back(Member(myint.get(), "c", 0));
  expected->members.push_back(Member(myint.get(), "b", 4));
  expected->members.push_back(Member(myint.get(), "c", 8));
  expected->members.push_back(Member(myint.get(), "a", 12));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, DuplicateMemberNames) {
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

  Flattener flattener;
  flattener.flatten({classA.get()});

  auto expected = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12); // TODO check size is tested
  // TODO duplicate names for "c" member variables
  expected->members.push_back(Member(myint.get(), "n", 0));
  expected->members.push_back(Member(myint.get(), "n", 4));
  expected->members.push_back(Member(myint.get(), "n", 8));

  EXPECT_EQ_CLASS(*classA, *expected);
}

TEST(TypeFlattenerTest, ParentsNotFlattened) {
  // Original:
  //   class C { int c; };
  //   class B : C { int b; };
  //   class A : B { int a; };
  //
  // Not flattened:
  //   class B : C { int b; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 12);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->parents.push_back(Parent(classC.get(), 0));
  classB->members.push_back(Member(myint.get(), "b", 4));

  classA->parents.push_back(Parent(classB.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 8));

  // When we request to flatten classA, check that parent classes are not
  // flattened
  auto expected = *classB;

  Flattener flattener;
  flattener.flatten({classA.get()});

  EXPECT_EQ_CLASS(*classB, expected);
}

TEST(TypeFlattenerTest, MembersNotFlattened) {
  // Original:
  //   class C { int c; };
  //   class B : C { int b; };
  //   class A : C { int a; B b; };
  //
  // Not flattened:
  //   class B : C { int b; };
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 16);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 8);
  auto classC = std::make_unique<Class>(Class::Kind::Class, "ClassC", 4);

  classC->members.push_back(Member(myint.get(), "c", 0));

  classB->parents.push_back(Parent(classC.get(), 0));
  classB->members.push_back(Member(myint.get(), "b", 4));

  classA->parents.push_back(Parent(classC.get(), 0));
  classA->members.push_back(Member(myint.get(), "a", 4));
  classA->members.push_back(Member(classB.get(), "b", 8));

  // When we request to flatten classA, check that classes used as members
  // variables are not flattened
  auto expected = *classB;

  Flattener flattener;
  flattener.flatten({classA.get()});

  EXPECT_EQ_CLASS(*classB, expected);
}

// TODO
// inherit from union
// typedefs
