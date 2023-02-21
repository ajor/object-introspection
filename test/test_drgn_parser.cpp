#include <regex>
#include <gtest/gtest.h>

#include "type_graph/drgn_parser.h"
#include "type_graph/printer.h"
#include "type_graph/type_graph.h"
#include "type_graph/types.h"

#include "SymbolService.h"
// TODO needed?:
#include "OIParser.h"
#include "ContainerInfo.h"

using namespace type_graph;

// TODO setup google logging for tests

// TODO put these in a header file
void EXPECT_EQ_TYPE(const Type *actual, const Type *expected);
void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected);

void test(std::string_view function, std::string_view expected) {
  SymbolService symbols{TARGET_EXE_PATH};
  irequest req{"entry", std::string{function}, "arg0"};
  auto drgnRoot = symbols.getRootType(req);

  TypeGraph typeGraph;
  // TODO more container types, with various template parameter options
  std::vector<ContainerInfo> containers = {
    ContainerInfo{
      "std::vector<",
      std::regex{"^std::vector<"},
      1,
      SEQ_TYPE,
      "vector",
      {"namespace std"},
      {},
      1,
      {}
    },
    ContainerInfo{
      "std::vector<",
      std::regex{"^std::vector<"},
      1,
      SEQ_TYPE,
      "vector",
      {"namespace std"},
      {},
      1,
      {}
    },
  };
  DrgnParser drgnParser(typeGraph, containers);
  Type *type = drgnParser.parse(drgnRoot->type.type);

  std::stringstream out;
  Printer printer(out);
  printer.print(*type);

  // TODO standardise expected-actual order
  expected.remove_prefix(1); // Remove initial '\n'
  EXPECT_EQ(expected, out.str());
}

TEST(DrgnParserTest, SimpleStruct) {
  test("oid_test_case_simple_struct", R"(
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

TEST(DrgnParserTest, SimpleClass) {
  test("oid_test_case_simple_class", R"(
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

TEST(DrgnParserTest, SimpleUnion) {
  test("oid_test_case_simple_union", R"(
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
  test("oid_test_case_inheritance_access_public", R"(
[0] Pointer
[1]   Class: Public (12)
        Parent (0)
[2]       Class: Base (8)
            Member: base_int (4)
              Primitive: int32_t
        Member: public_int (8)
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, Container) {
  test("oid_test_case_std_vector_int_empty", R"(
[0] Pointer
[1]   Container: std::vector
        Param
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, Enum) {
  test("oid_test_case_enums_scoped", R"(
    Enum: ScopedEnum (4)
)");
}

TEST(DrgnParserTest, EnumInt8) {
  test("oid_test_case_enums_scoped_int8", R"(
    Enum: ScopedEnumInt8 (1)
)");
}

TEST(DrgnParserTest, UnscopedEnum) {
  test("oid_test_case_enums_unscoped", R"(
    Enum: UNSCOPED_ENUM (4)
)");
}

TEST(DrgnParserTest, Typedef) { // TODO
  test("TestTypedef", R"(
[0] Typedef: UInt64
[1]   Typedef: uint64_t
[2]     Typedef: __uint64_t
          Primitive: uint64_t
)");
}

TEST(DrgnParserTest, Using) { // TODO
  test("TestUsing", R"(
[0] Pointer
[1]   Typedef: IntVector
[2]     Container: std::vector
          Param
            Primitive: int32_t
)");
}

TEST(DrgnParserTest, Cycle) {
  test("oid_test_case_cycles_raw_ptr", R"(
[0] Pointer
[1]   Struct: RawNode (16)
        Member: value (0)
          Primitive: int64_t
        Member: next (8)
[2]       Pointer
            [1]
)");
}

TEST(DrgnParserTest, Array) {
  test("TestArray", R"(
[0] Pointer
[1]   Struct: ArrayStruct (72)
        Member: intArray (0)
[2]       Array: (5)
            Primitive: int32_t
        Member: structArray (24)
[3]       Array: (3)
[4]         Struct: SimpleStruct (16)
              Member: a (0)
                Primitive: int32_t
              Member: b (4)
                Primitive: int8_t
              Member: c (8)
                Primitive: int64_t
        Member: charArray (72)
[5]       Array: (0)
            Primitive: int8_t
)");
}

TEST(DrgnParserTest, ClassTemplateInt) {
  test("TestClassTemplateInt", R"(
[0] Pointer
[1]   Class: TemplatedClass1_int32_t_ (4)
        Param
          Primitive: int32_t
        Member: templatedValue (0)
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, ClassTemplateVector) {
  test("TestClassTemplateVector", R"(
[0] Pointer
[1]   Class: TemplatedClass1_std::vector_ (24)
        Param
[2]       Container: std::vector
            Param
              Primitive: int32_t
        Member: templatedValue (0)
          [2]
        Function: TemplatedClass1 (virtuality: 0)
        Function: ~TemplatedClass1 (virtuality: 0)
)");
}

TEST(DrgnParserTest, ClassTemplateSimpleStructInt) {
  test("TestClassTemplateSimpleStructInt", R"(
[0] Pointer
[1]   Class: TemplatedClass2_SimpleStruct_int32_t_ (24)
        Param
[2]       Struct: SimpleStruct (16)
            Member: a (0)
              Primitive: int32_t
            Member: b (4)
              Primitive: int8_t
            Member: c (8)
              Primitive: int64_t
        Param
          Primitive: int32_t
        Member: tc1 (0)
[3]       Class: TemplatedClass1_SimpleStruct_ (16)
            Param
              [2]
            Member: templatedValue (0)
              [2]
        Member: arr (16)
[4]       Array: (2)
            Primitive: int32_t
)");
}

TEST(DrgnParserTest, ClassFunctions) {
  test("TestClassFunctions", R"(
[0] Pointer
[1]   Class: ClassFunctions (4)
        Member: memberA (0)
          Primitive: int32_t
        Function: foo (virtuality: 0)
        Function: bar (virtuality: 0)
)");
}
