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
class AlignmentCalc final : public LazyVisitor {
public:
  static Pass createPass();

  void calculateAlignments(const std::vector<std::reference_wrapper<Type>> &types);

  void visit(Class &c) override;

private:
  void calculateAlignment(Type &type);
};

} // namespace type_graph
