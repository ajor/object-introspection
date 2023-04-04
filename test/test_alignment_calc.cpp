#include <gtest/gtest.h>

#include "type_graph/AlignmentCalc.h"

using namespace type_graph;

TEST(AlignmentCalcTest, PrimitiveMembers) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 16);
  auto myint8 = std::make_unique<Primitive>(Primitive::Kind::Int8);
  auto myint64 = std::make_unique<Primitive>(Primitive::Kind::Int64);
  myclass->members.push_back(Member(myint8.get(), "n", 0));
  myclass->members.push_back(Member(myint64.get(), "n", 0));

  AlignmentCalc calc;
  calc.calculateAlignments({*myclass});

  EXPECT_EQ(myclass->align(), 8);
}

TEST(AlignmentCalcTest, StructMembers) {
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 8);
  auto myint32 = std::make_unique<Primitive>(Primitive::Kind::Int32);
  mystruct->members.push_back(Member(myint32.get(), "n1", 0));
  mystruct->members.push_back(Member(myint32.get(), "n2", 4));

  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 12);
  auto myint8 = std::make_unique<Primitive>(Primitive::Kind::Int8);
  myclass->members.push_back(Member(myint8.get(), "n", 0));
  myclass->members.push_back(Member(mystruct.get(), "s", 4));

  AlignmentCalc calc;
  calc.calculateAlignments({*myclass});

  EXPECT_EQ(myclass->align(), 4);
}

TEST(AlignmentCalcTest, StructInContainer) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 16);
  auto myint8 = std::make_unique<Primitive>(Primitive::Kind::Int8);
  auto myint64 = std::make_unique<Primitive>(Primitive::Kind::Int64);
  myclass->members.push_back(Member(myint8.get(), "n", 0));
  myclass->members.push_back(Member(myint64.get(), "n", 0));

  auto mycontainer = std::make_unique<Container>(ContainerInfo{}, 8);
  mycontainer->templateParams.push_back(myclass.get());

  AlignmentCalc calc;
  calc.calculateAlignments({*mycontainer});

  EXPECT_EQ(myclass->align(), 8);
}
