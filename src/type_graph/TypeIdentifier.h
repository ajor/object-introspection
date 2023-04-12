#pragma once

#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

class TypeGraph;

/*
 * TODO Pass Name
 *
 * TODO description
 */
class TypeIdentifier : public RecursiveVisitor {
public:
  static Pass createPass(const std::vector<ContainerInfo> &containers);

  TypeIdentifier(TypeGraph &typeGraph,
      const std::vector<ContainerInfo> &containers)
    : typeGraph_(typeGraph), containers_(containers) { }

  void visit(Type &type) override;
  void visit(Container &c) override;

private:
  std::unordered_set<Type*> visited_;
  const std::vector<ContainerInfo> &containers_;
  TypeGraph &typeGraph_;
};

} // namespace type_graph
