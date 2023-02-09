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

TEST(DrgnParserTest, Container) {
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
          TypeDef: __allocator_base<SimpleStruct>
)");
}
