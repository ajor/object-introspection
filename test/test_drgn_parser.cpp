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
  expected.remove_prefix(1); // Remove initial '\n'
  EXPECT_EQ(expected, out.str());
}

TEST(DrgnParserTest, SimpleClass) {
  test("TestSimpleClass", R"(
[0] Pointer
[1]   Class: SimpleClass (16)
        Member: a (0)
          Primitive: int32_t
        Member: b (4)
          Primitive: int8_t
        Member: c (8)
          Primitive: int64_t
)");
}

TEST(DrgnParserTest, SimpleStruct) {
  test("TestSimpleStruct", R"(
[0] Pointer
[1]   Struct: SimpleStruct (16)
        Member: a (0)
          Primitive: int32_t
        Member: b (4)
          Primitive: int8_t
        Member: c (8)
          Primitive: int64_t
)");
}

TEST(DrgnParserTest, SimpleUnion) {
  test("TestSimpleUnion", R"(
[0] Pointer
[1]   Union: SimpleUnion (8)
        Member: a (0)
          Primitive: int32_t
        Member: b (0)
          Primitive: int8_t
        Member: c (0)
          Primitive: int64_t
)");
}

TEST(DrgnParserTest, Inheritance) {
  test("TestInheritance", R"(
[0] Pointer
[1]   Class: InheritanceChild (12)
        Parent (0)
[2]       Class: InheritanceMiddle (8)
            Parent (0)
[3]           Class: InheritanceBase (4)
                Member: a (0)
                  Primitive: int32_t
            Member: b (4)
              Primitive: int32_t
        Member: c (8)
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, Container) { // TODO strip template parameters from containers
  test("TestContainer", R"(
[0] Pointer
[1]   Container: std::vector
        Param
[2]       Struct: SimpleStruct (16)
            Member: a (0)
              Primitive: int32_t
            Member: b (4)
              Primitive: int8_t
            Member: c (8)
              Primitive: int64_t
        Param
[3]       Class: allocator_SimpleStruct_ (1)
            Parent (0)
[4]           Typedef: __allocator_base<SimpleStruct>
[5]             Class: new_allocator_SimpleStruct_ (1)
)");
}

TEST(DrgnParserTest, Enum) {
  test("TestEnum", R"(
    Enum: MyEnum (4)
)");
}

TEST(DrgnParserTest, EnumInt8) {
  test("TestEnumInt8", R"(
    Enum: MyEnumInt8 (1)
)");
}

TEST(DrgnParserTest, UnscopedEnum) {
  test("TestUnscopedEnum", R"(
    Enum: MyUnscopedEnum (4)
)");
}

TEST(DrgnParserTest, Typedef) {
  test("TestTypedef", R"(
[0] Typedef: UInt64
[1]   Typedef: uint64_t
[2]     Typedef: __uint64_t
          Primitive: uint64_t
)");
}

TEST(DrgnParserTest, Using) { // TODO allocator param should not be here
  test("TestUsing", R"(
[0] Pointer
[1]   Typedef: IntVector
[2]     Container: std::vector
          Param
            Primitive: int32_t
          Param
[3]         Class: allocator_int32_t_ (1)
              Parent (0)
[4]             Typedef: __allocator_base<int>
[5]               Class: new_allocator_int32_t_ (1)
)");
}

TEST(DrgnParserTest, Cycle) {
  test("TestCycle", R"(
[0] Pointer
[1]   Struct: CyclicalNode (16)
        Member: next (0)
[2]       Pointer
            [1]
        Member: val (8)
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, Array) {
  test("TestArray", R"(
[0] Pointer
[1]   Struct: ArrayStruct (72)
        Member: int_array (0)
[2]       Array: (5)
            Primitive: int32_t
        Member: struct_array (24)
[3]       Array: (3)
[4]         Struct: SimpleStruct (16)
              Member: a (0)
                Primitive: int32_t
              Member: b (4)
                Primitive: int8_t
              Member: c (8)
                Primitive: int64_t
)");
}
