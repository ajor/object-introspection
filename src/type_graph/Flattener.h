#pragma once

#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * Flattener
 *
 * Flattens classes by removing parents and adding their members directly into
 * derived classes.
 */
class Flattener : public Visitor {
public:
  static Pass createPass();

  void flatten(std::vector<std::reference_wrapper<Type>> types);
  void visit(Type &type);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Array &a) override;
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;

private:
  std::unordered_set<Type*> visited_;
  std::vector<Member> flattened_members_;
  std::vector<uint64_t> offset_stack_;
};

} // namespace type_graph
