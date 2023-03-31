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

// TODO setup google logging for tests so it doesn't appear on terminal by default

class DrgnParserTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    symbols_ = new SymbolService{TARGET_EXE_PATH};
  }

  static void TearDownTestSuite() {
    // TODO How do we free drgn memory?
  }

  void test(std::string_view function, std::string_view expected);

  static SymbolService *symbols_;
};

SymbolService *DrgnParserTest::symbols_ = nullptr;

void DrgnParserTest::test(std::string_view function, std::string_view expected) {
  irequest req{"entry", std::string{function}, "arg0"};
  auto drgnRoot = symbols_->getRootType(req);

  TypeGraph typeGraph;
  // TODO more container types, with various template parameter options
  ContainerInfo std_vector;
  std_vector.typeName = "std::vector";
  std_vector.matcher = "^std::vector<";
  std_vector.ctype = SEQ_TYPE;
  std_vector.stubTemplateParams = {1};

  std::vector<ContainerInfo> containers;
  containers.emplace_back(std::move(std_vector));

  DrgnParser drgnParser(typeGraph, containers, true); // TODO test chaseRawPointers
  Type *type = drgnParser.parse(drgnRoot->type.type);

  std::stringstream out;
  Printer printer(out);
  printer.print(*type);

  // TODO standardise expected-actual order
  expected.remove_prefix(1); // Remove initial '\n'
  EXPECT_EQ(expected, out.str());
}

TEST_F(DrgnParserTest, SimpleStruct) {
  test("oid_test_case_simple_struct", R"(
[0] Pointer
[1]   Struct: SimpleStruct (size: 16)
        Member: a (offset: 0)
          Primitive: int32_t
        Member: b (offset: 4)
          Primitive: int8_t
        Member: c (offset: 8)
          Primitive: int64_t
)");
}

TEST_F(DrgnParserTest, SimpleClass) {
  test("oid_test_case_simple_class", R"(
[0] Pointer
[1]   Class: SimpleClass (size: 16)
        Member: a (offset: 0)
          Primitive: int32_t
        Member: b (offset: 4)
          Primitive: int8_t
        Member: c (offset: 8)
          Primitive: int64_t
)");
}

TEST_F(DrgnParserTest, SimpleUnion) {
  test("oid_test_case_simple_union", R"(
[0] Pointer
[1]   Union: SimpleUnion (size: 8)
        Member: a (offset: 0)
          Primitive: int32_t
        Member: b (offset: 0)
          Primitive: int8_t
        Member: c (offset: 0)
          Primitive: int64_t
)");
}

TEST_F(DrgnParserTest, Inheritance) {
  test("oid_test_case_inheritance_access_public", R"(
[0] Pointer
[1]   Class: Public (size: 8)
        Parent (offset: 0)
[2]       Class: Base (size: 4)
            Member: base_int (offset: 0)
              Primitive: int32_t
        Member: public_int (offset: 4)
          Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, InheritanceMultiple) {
  test("oid_test_case_inheritance_multiple_a", R"(
[0] Pointer
[1]   Struct: Derived_2 (size: 24)
        Parent (offset: 0)
[2]       Struct: Base_1 (size: 4)
            Member: a (offset: 0)
              Primitive: int32_t
        Parent (offset: 4)
[3]       Struct: Derived_1 (size: 12)
            Parent (offset: 0)
[4]           Struct: Base_2 (size: 4)
                Member: b (offset: 0)
                  Primitive: int32_t
            Parent (offset: 4)
[5]           Struct: Base_3 (size: 4)
                Member: c (offset: 0)
                  Primitive: int32_t
            Member: d (offset: 8)
              Primitive: int32_t
        Parent (offset: 16)
[6]       Struct: Base_4 (size: 4)
            Member: e (offset: 0)
              Primitive: int32_t
        Member: f (offset: 20)
          Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, Container) {
  test("oid_test_case_std_vector_int_empty", R"(
[0] Pointer
[1]   Container: std::vector
        Param
          Primitive: int32_t
        Param
          Dummy (size: 1)
)");
}
// TODO test vector with custom allocator

TEST_F(DrgnParserTest, Enum) {
  test("oid_test_case_enums_scoped", R"(
    Enum: ScopedEnum (size: 4)
)");
}

TEST_F(DrgnParserTest, EnumInt8) {
  test("oid_test_case_enums_scoped_int8", R"(
    Enum: ScopedEnumInt8 (size: 1)
)");
}

TEST_F(DrgnParserTest, UnscopedEnum) {
  test("oid_test_case_enums_unscoped", R"(
    Enum: UNSCOPED_ENUM (size: 4)
)");
}

TEST_F(DrgnParserTest, Typedef) {
  test("oid_test_case_typedefs_c_style", R"(
[0] Typedef: UInt64
[1]   Typedef: uint64_t
[2]     Typedef: __uint64_t
          Primitive: uint64_t
)");
}

TEST_F(DrgnParserTest, Using) {
  test("oid_test_case_typedefs_using", R"(
[0] Pointer
[1]   Typedef: IntVector
[2]     Container: std::vector
          Param
            Primitive: int32_t
          Param
            Dummy (size: 1)
)");
}

TEST_F(DrgnParserTest, ArrayMember) {
  test("oid_test_case_arrays_member_int10", R"(
[0] Pointer
[1]   Struct: Foo10 (size: 40)
        Member: arr (offset: 0)
[2]       Array: (length: 10)
            Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, ArrayRef) {
  test("oid_test_case_arrays_ref_int10", R"(
[0] Pointer
[1]   Array: (length: 10)
        Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, ArrayDirect) {
  test("oid_test_case_arrays_direct_int10", R"(
[0] Pointer
      Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, Pointer) {
  GTEST_SKIP() << "Need to setup chase-follow-pointers setting";
  test("oid_test_case_pointers_struct_vector_ptr", R"(
[0] Pointer
[1]   Struct: VectorPtr (size: 8)
        Member: vec (offset: 0)
[2]       Pointer
[3]         Container: std::vector
              Param
                Primitive: int32_t
              Param
                Dummy (size: 1)
)");
}

TEST_F(DrgnParserTest, PointerNoFollow) {
  test("oid_test_case_pointers_struct_vector_ptr", R"(
[0] Pointer
[1]   Struct: VectorPtr (size: 8)
        Member: vec (offset: 0)
          Primitive: uintptr_t
)");
}

TEST_F(DrgnParserTest, PointerIncomplete) {
  test("oid_test_case_pointers_incomplete_raw", R"(
    Primitive: uintptr_t
)");
}

TEST_F(DrgnParserTest, Cycle) { // TODO switch this integration test to use "int" instead uint64_t", to remove typedefs
  GTEST_SKIP() << "Cycle test needs updating";
  test("oid_test_case_cycles_raw_ptr", R"(
[0] Pointer
[1]   Struct: RawNode (size: 16)
        Member: value (offset: 0)
          Primitive: int64_t
        Member: next (offset: 8)
[2]       Pointer
            [1]
)");
}

TEST_F(DrgnParserTest, ClassTemplateInt) {
  test("oid_test_case_templates_int", R"(
[0] Pointer
[1]   Class: TemplatedClass1<int> (size: 4)
        Param
          Primitive: int32_t
        Member: val (offset: 0)
          Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, ClassTemplateVector) {
  test("oid_test_case_templates_vector", R"(
[0] Pointer
[1]   Class: TemplatedClass1<std::vector<int, std::allocator<int> > > (size: 24)
        Param
[2]       Container: std::vector
            Param
              Primitive: int32_t
            Param
              Dummy (size: 1)
        Member: val (offset: 0)
          [2]
        Function: ~TemplatedClass1 (virtuality: 0)
        Function: TemplatedClass1 (virtuality: 0)
)");
}

TEST_F(DrgnParserTest, ClassTemplateTwo) {
  test("oid_test_case_templates_two", R"(
[0] Pointer
[1]   Class: TemplatedClass2<ns_templates::Foo, int> (size: 12)
        Param
[2]       Struct: Foo (size: 8)
            Member: a (offset: 0)
              Primitive: int32_t
            Member: b (offset: 4)
              Primitive: int32_t
        Param
          Primitive: int32_t
        Member: tc1 (offset: 0)
[3]       Class: TemplatedClass1<ns_templates::Foo> (size: 8)
            Param
              [2]
            Member: val (offset: 0)
              [2]
        Member: val2 (offset: 8)
          Primitive: int32_t
)");
}

TEST_F(DrgnParserTest, ClassTemplateValue) {
  test("oid_test_case_templates_value", R"(
[0] Pointer
[1]   Struct: TemplatedClassVal<3> (size: 12)
        Param
          Value: 3
        Member: arr (offset: 0)
[2]       Array: (length: 3)
            Primitive: int32_t
)");
}

// TODO
//TEST_F(DrgnParserTest, ClassFunctions) {
//  test("TestClassFunctions", R"(
//[0] Pointer
//[1]   Class: ClassFunctions (size: 4)
//        Member: memberA (offset: 0)
//          Primitive: int32_t
//        Function: foo (virtuality: 0)
//        Function: bar (virtuality: 0)
//)");
//}

TEST_F(DrgnParserTest, Alignment) {
  GTEST_SKIP() << "Alignment not reported by drgn yet";
  test("oid_test_case_alignment_struct", R"(
[0] Pointer
[1]   Struct: Align16 (size: 16, align: 16)
        Member: c (offset: 0)
          Primitive: int8_t
)");
}
// TODO member alignment
