#pragma once

#include <unordered_set>
#include <vector>

#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * RequiredTypeCollector
 *
 * Walks the type graph and collects all types which must appear in the
 * generated C++ code to fully define a provided list of types.
 */
class RequiredTypeCollector : public Visitor {
public:
  // TODO determine whether we need classes and typedefs in a single list or separately
  std::vector<Type*> collect(const std::vector<Type*> &types);
  std::vector<Class*> classes() const { return required_classes_; };

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  void collect_type(Type *type);
  bool chasePointer() const;

  std::unordered_set<Type*> visited_;
  std::vector<Type*> required_types_;
  std::vector<Class*> required_classes_;
  int depth;
};

} // namespace type_graph
