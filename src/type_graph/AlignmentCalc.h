#pragma once

#include <functional>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * AlignmentCalc
 *
 * TODO comment about class (and all other passes too)
 */
class AlignmentCalc final : public RecursiveVisitor {
public:
  static Pass createPass();

  void calculateAlignments(const std::vector<std::reference_wrapper<Type>> &types);

  void visit(Type &type) override;
  void visit(Class &c) override;
};

} // namespace type_graph
