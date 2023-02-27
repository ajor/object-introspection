#pragma once

#include <queue>
#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * TopoSorter
 *
 * Topologically sorts a list of types so that dependencies appear before
 * dependent types.
 */
class TopoSorter : public Visitor {
public:
  static Pass createPass();

  std::vector<std::reference_wrapper<Type>> sort(std::vector<std::reference_wrapper<Type>> types);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Primitive &p) override;
  void visit(Enum &e) override;
  void visit(Array &a) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;

private:
  std::unordered_set<Type*> visited_;
  std::vector<std::reference_wrapper<Type>> sortedTypes_;
  std::queue<std::reference_wrapper<Type>> typesToSort_;

  void sort_type(Type &type);
};

} // namespace type_graph
