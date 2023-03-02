#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

// TODO make all final
class NameGen final : public Visitor {
public:
  static Pass createPass();

  void generateNames(const std::vector<std::reference_wrapper<Type>> &types);

  void visit(Class &c) override;
  void visit(Container &c) override;

private:
  void nameType(Type &type);
  void removeTemplateParams(std::string &name);

  std::unordered_set<Type*> visited_;
  int n = 0;
};

} // namespace type_graph
