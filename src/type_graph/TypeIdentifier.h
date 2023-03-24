#pragma once

#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "TypeGraph.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * TODO Pass Name
 *
 * TODO description
 */
class TypeIdentifier : public Visitor {
public:
  static Pass createPass(const std::vector<ContainerInfo> &containers);

  TypeIdentifier(TypeGraph &typeGraph,
      const std::vector<ContainerInfo> &containers)
    : typeGraph_(typeGraph), containers_(containers) { }
  void visit(Type &type);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Array &a) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;

private:
  std::unordered_set<Type*> visited_;
  const std::vector<ContainerInfo> &containers_;
  TypeGraph &typeGraph_;

  template <typename T, typename ...Args>
  T *make_type(Args &&...args) {
    auto type_unique_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto type_raw_ptr = type_unique_ptr.get();
    typeGraph_.add(std::move(type_unique_ptr));
    return type_raw_ptr;
  }
};

} // namespace type_graph
