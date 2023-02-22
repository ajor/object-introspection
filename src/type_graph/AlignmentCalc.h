#pragma once

#include <vector>

#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * AlignmentCalc
 *
 * TODO comment about class (and all other passes too)
 */
class AlignmentCalc final : public Visitor {
public:
  void calculateAlignments(const std::vector<Type*> &types);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  void calculateAlignment(Type *type);
};

} // namespace type_graph
