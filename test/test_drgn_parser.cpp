#include <gtest/gtest.h>

#include "type_graph/drgn_parser.h"
#include "type_graph/type_graph.h"
#include "type_graph/types.h"

// TODO these shouldn't be needed for testing this unit
#include "SymbolService.h"
#include "OICodeGen.h"
#include "OIParser.h"

using namespace type_graph;

// TODO put these in a header file
void EXPECT_EQ_TYPE(const Type *actual, const Type *expected);
void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected);

Type *setupTest(TypeGraph &typeGraph, std::string_view function) {
  SymbolService symbols{"./target_prog"};
  irequest req{"entry", std::string{function}, "arg0"};
  auto drgnRoot = OICodeGen::getRootType(symbols, req);

  DrgnParser drgnParser(typeGraph);
  Type *asdf = drgnParser.parse(drgnRoot->type.type);
  return asdf;
}

TEST(DrgnParserTest, SimpleStruct) {
  TypeGraph typeGraph;
  auto *actual = setupTest(typeGraph, "TestSimpleStruct");
  auto simpleStruct = std::make_unique<Class>(Class::Kind::Class, "SimpleStruct", 13);
  auto simpleStructPtr = std::make_unique<Pointer>(simpleStruct.get());

  EXPECT_EQ_TYPE(actual, simpleStructPtr.get());
}
