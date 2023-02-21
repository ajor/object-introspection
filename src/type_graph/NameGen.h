#pragma once

#include <unordered_set>
#include <vector>

#include "Types.h"
#include "Visitor.h"

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
  void nameType(Type *type);
  void removeTemplateParams(std::string &name);

  std::unordered_set<Type*> visited_;
  int n = 0;
};

} // namespace type_graph
