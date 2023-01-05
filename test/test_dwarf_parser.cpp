#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dwarf_parser.h"
#include "type_graph.h"
#include "types.h"

TEST(TopoSortTest, SingleType) {
  TypeGraph type_graph;
  DwarfParser p(type_graph);
  p.parse(root->type.type);
}
