#include <gtest/gtest.h>

#include "type_graph/drgn_parser.h"
#include "type_graph/printer.h"
#include "type_graph/type_graph.h"
#include "type_graph/types.h"

// TODO these shouldn't be needed for testing this unit
#include "SymbolService.h"
#include "OICodeGen.h"
#include "OIParser.h"

using namespace type_graph;

// TODO setup google logging for tests

// TODO put these in a header file
void EXPECT_EQ_TYPE(const Type *actual, const Type *expected);
void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected);

void test(std::string_view function, std::string_view expected) {
  // TODO turn this into an absolute path
  SymbolService symbols{"./target_prog"};
  irequest req{"entry", std::string{function}, "arg0"};
  auto drgnRoot = OICodeGen::getRootType(symbols, req);

  TypeGraph typeGraph;
  DrgnParser drgnParser(typeGraph);
  Type *type = drgnParser.parse(drgnRoot->type.type);

  std::stringstream out;
  Printer printer(out);
  printer.print(*type);

  // TODO standardise expected-actual order
  EXPECT_EQ(expected, out.str());
}

TEST(DrgnParserTest, SimpleStruct) {
  test("TestSimpleStruct",
       R"(Pointer
  Class: SimpleStruct (12)
    Member: a (0)
      Primitive: int32_t
    Member: b (4)
      Primitive: int32_t
    Member: c (8)
      Primitive: int32_t
)");
}

TEST(DrgnParserTest, Inheritance) {
  test("TestInheritance",
       R"(Pointer
  Class: InheritanceChild (12)
    Parent (0)
      Class: InheritanceMiddle (8)
        Parent (0)
          Class: InheritanceBase (4)
            Member: a (0)
              Primitive: int32_t
        Member: b (4)
          Primitive: int32_t
    Member: c (8)
      Primitive: int32_t
)");
}

TEST(DrgnParserTest, Container) { // TODO strip template parameters from containers
  test("TestContainer",
       R"(Pointer
  Container: std::vector
    Param
      Class: SimpleStruct (12)
        Member: a (0)
          Primitive: int32_t
        Member: b (4)
          Primitive: int32_t
        Member: c (8)
          Primitive: int32_t
    Param
      Class: allocator_SimpleStruct_ (1)
        Parent (0)
          Typedef: __allocator_base<SimpleStruct>
            Class: new_allocator_SimpleStruct_ (1)
)");
}

TEST(DrgnParserTest, Enum) {
  test("TestEnum",
       R"(Enum: MyEnum (4)
)");
}

TEST(DrgnParserTest, EnumInt8) {
  test("TestEnumInt8",
       R"(Enum: MyEnumInt8 (1)
)");
}

TEST(DrgnParserTest, UnscopedEnum) {
  test("TestUnscopedEnum",
       R"(Enum: MyUnscopedEnum (4)
)");
}

TEST(DrgnParserTest, Typedef) {
  test("TestTypedef",
       R"(Typedef: UInt64
  Typedef: uint64_t
    Typedef: __uint64_t
      Primitive: uint64_t
)");
}

TEST(DrgnParserTest, Using) { // TODO allocator param should not be here
  test("TestUsing",
       R"(Pointer
  Typedef: IntVector
    Container: std::vector
      Param
        Primitive: int32_t
      Param
        Class: allocator_int32_t_ (1)
          Parent (0)
            Typedef: __allocator_base<int>
              Class: new_allocator_int32_t_ (1)
)");
}
