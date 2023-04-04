#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * AlignmentCalc
 *
 * Calculates alignment information for types TODO finish comment
 * TODO is this pass even needed??
 */
class AlignmentCalc final : public RecursiveVisitor {
public:
  static Pass createPass();

  void calculateAlignments(const std::vector<std::reference_wrapper<Type>> &types);

  void visit(Type &type) override;
  void visit(Class &c) override;

private:
  std::unordered_set<Type*> visited_;
};

} // namespace type_graph
