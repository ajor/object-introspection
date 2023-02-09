#include <gtest/gtest.h>

#include "drgn_parser.h"
#include "type_graph.h"
#include "types.h"

// TODO these shouldn't be needed for testing this unit
#include "SymbolService.h"
#include "OICodeGen.h"

void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected) {
  EXPECT_EQ(actual.size(), expected.size());
  for (std::size_t i=0; i<std::min(actual.size(), expected.size()); i++) {
    EXPECT_EQ(actual[i]->name(), expected[i]->name());
  }
}

TEST(DrgnParserTest, SingleType) {
  SymbolService symbols{"./a.out"};
  irequest req{"entry", "TestSimpleStruct", "arg0"};
  auto drgnRoot = OICodeGen::getRootType(symbols, req);

  TypeGraph type_graph;
  DrgnParser drgnParser(type_graph);
  Type *rootType = drgnParser.parse(drgnRoot->type.type);
}
