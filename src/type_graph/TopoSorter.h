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
class TopoSorter : public RecursiveVisitor {
public:
  static Pass createPass();

  void sort(const std::vector<std::reference_wrapper<Type>> &types);
  const std::vector<std::reference_wrapper<Type>> &sortedTypes() const;

  void visit(Type &type) override;
  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;

private:
  std::unordered_set<Type*> visited_;
  std::vector<std::reference_wrapper<Type>> sortedTypes_;
  std::queue<std::reference_wrapper<Type>> typesToSort_;
};

} // namespace type_graph
