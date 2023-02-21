#pragma once

#include <vector>

#include "types.h"
#include "visitor.h"

namespace type_graph {

// TODO make all final
class NameGen final : public Visitor {
public:
  void generateNames(const std::vector<Type*> &types);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  int n = 0;
};

} // namespace type_graph
