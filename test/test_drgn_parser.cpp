#include <regex>
#include <gtest/gtest.h>

#include "type_graph/DrgnParser.h"
#include "type_graph/Printer.h"
#include "type_graph/TypeGraph.h"
#include "type_graph/Types.h"

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
[1]   Class: Public (8)
        Parent (0)
[2]       Class: Base (4)
            Member: base_int (0)
              Primitive: int32_t
        Member: public_int (4)
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
// TODO test vector with custom allocator

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

TEST(DrgnParserTest, Typedef) {
  test("oid_test_case_typedefs_c_style", R"(
[0] Typedef: UInt64
[1]   Typedef: uint64_t
[2]     Typedef: __uint64_t
          Primitive: uint64_t
)");
}

TEST(DrgnParserTest, Using) {
  test("oid_test_case_typedefs_using", R"(
[0] Pointer
[1]   Typedef: IntVector
[2]     Container: std::vector
          Param
            Primitive: int32_t
)");
}

TEST(DrgnParserTest, Cycle) { // TODO switch this integration test to use "int" instead uint64_t", to remove typedefs
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

TEST(DrgnParserTest, ArrayMember) {
  test("oid_test_case_arrays_member_int10", R"(
[0] Pointer
[1]   Struct: Foo10 (40)
        Member: arr (0)
[2]       Array: (10)
            Primitive: int32_t
)");
}

TEST(DrgnParserTest, ArrayRef) {
  test("oid_test_case_arrays_ref_int10", R"(
[0] Pointer
[1]   Array: (10)
        Primitive: int32_t
)");
}

TEST(DrgnParserTest, ArrayDirect) {
  test("oid_test_case_arrays_direct_int10", R"(
[0] Pointer
      Primitive: int32_t
)");
}

TEST(DrgnParserTest, ClassTemplateInt) {
  test("oid_test_case_templates_int", R"(
[0] Pointer
[1]   Class: TemplatedClass1<int> (4)
        Param
          Primitive: int32_t
        Member: val (0)
          Primitive: int32_t
)");
}

TEST(DrgnParserTest, ClassTemplateVector) {
  test("oid_test_case_templates_vector", R"(
[0] Pointer
[1]   Class: TemplatedClass1<std::vector<int, std::allocator<int> > > (24)
        Param
[2]       Container: std::vector
            Param
              Primitive: int32_t
        Member: val (0)
          [2]
        Function: ~TemplatedClass1 (virtuality: 0)
        Function: TemplatedClass1 (virtuality: 0)
)");
}

TEST(DrgnParserTest, ClassTemplateTwo) {
  test("oid_test_case_templates_two", R"(
[0] Pointer
[1]   Class: TemplatedClass2<ns_templates::Foo, int> (12)
        Param
[2]       Struct: Foo (8)
            Member: a (0)
              Primitive: int32_t
            Member: b (4)
              Primitive: int32_t
        Param
          Primitive: int32_t
        Member: tc1 (0)
[3]       Class: TemplatedClass1<ns_templates::Foo> (8)
            Param
              [2]
            Member: val (0)
              [2]
        Member: val2 (8)
          Primitive: int32_t
)");
}

// TODO
//TEST(DrgnParserTest, ClassTemplateValue) {
//  test("oid_test_case_templates_value", R"(
//[0] Pointer
//[1]   Class: TemplatedClassVal<3> (12)
//        Param
//          Value: 3
//        Member: arr (0)
//[2]       Array: (3)
//            Primitive: int32_t
//)");
//}

// TODO
//TEST(DrgnParserTest, ClassFunctions) {
//  test("TestClassFunctions", R"(
//[0] Pointer
//[1]   Class: ClassFunctions (4)
//        Member: memberA (0)
//          Primitive: int32_t
//        Function: foo (virtuality: 0)
//        Function: bar (virtuality: 0)
//)");
//}
