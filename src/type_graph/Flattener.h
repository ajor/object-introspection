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
class Flattener : public RecursiveVisitor {
public:
  static Pass createPass();

  void flatten(std::vector<std::reference_wrapper<Type>> &types); // TODO make sure all passes take references, not vectors by value

  void visit(Type &type) override;
  void visit(Class &c) override;
  void visit(Container &c) override;

private:
  std::unordered_set<Type*> visited_;
  std::vector<Member> flattened_members_;
  std::vector<uint64_t> offset_stack_;
};

} // namespace type_graph
