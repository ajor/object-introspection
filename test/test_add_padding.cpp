#include <gtest/gtest.h>

#include "type_graph/AddPadding.h"
#include "type_graph/PassManager.h"
#include "type_graph/Printer.h"
#include "type_graph/TypeGraph.h"
#include "type_graph/Types.h"

using namespace type_graph;

void test(Pass pass, std::vector<std::reference_wrapper<Type>> types, std::string_view expected) {
  TypeGraph typeGraph;
  for (const auto& type : types) {
    typeGraph.addRoot(type);
  }

  pass.run(typeGraph);

  std::stringstream out;
  Printer printer(out);

  for (const auto &type : types) {
    printer.print(type);
  }

  expected.remove_prefix(1); // Remove initial '\n'
  EXPECT_EQ(expected, out.str());
}

TEST(AddPaddingTest, BetweenMembers) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 16);
  auto myint8 = std::make_unique<Primitive>(Primitive::Kind::Int8);
  auto myint64 = std::make_unique<Primitive>(Primitive::Kind::Int64);
  myclass->members.push_back(Member(myint8.get(), "n1", 0));
  myclass->members.push_back(Member(myint64.get(), "n2", 8));

  test(AddPadding::createPass(), {*myclass}, R"(
[0] Class: MyClass (size: 16)
      Member: n1 (offset: 0)
        Primitive: int8_t
      Member: padding (offset: 1)
[1]     Array: (length: 7)
          Primitive: int8_t
      Member: n2 (offset: 8)
        Primitive: int64_t
)");
}

TEST(AddPaddingTest, AtEnd) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 16);
  auto myint8 = std::make_unique<Primitive>(Primitive::Kind::Int8);
  auto myint64 = std::make_unique<Primitive>(Primitive::Kind::Int64);
  myclass->members.push_back(Member(myint64.get(), "n1", 0));
  myclass->members.push_back(Member(myint8.get(), "n2", 8));

  test(AddPadding::createPass(), {*myclass}, R"(
[0] Class: MyClass (size: 16)
      Member: n1 (offset: 0)
        Primitive: int64_t
      Member: n2 (offset: 8)
        Primitive: int8_t
      Member: padding (offset: 9)
[1]     Array: (length: 7)
          Primitive: int8_t
)");
}
