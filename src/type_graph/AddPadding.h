#pragma once

#include <functional>
#include <unordered_set>
#include <vector>

#include "PassManager.h"
#include "TypeGraph.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * AddPadding
 *
 * Adds members to classes to represent padding. This is necessary until we have
 * complete alignment information from DWARF, otherwise our classes may be
 * undersized.
 */
class AddPadding final : public RecursiveVisitor {
public:
  static Pass createPass();

  explicit AddPadding(TypeGraph& typeGraph) : typeGraph_(typeGraph) { }

  void visit(Type &type) override;
  void visit(Class &c) override;

private:
  std::unordered_set<Type*> visited_;
  TypeGraph &typeGraph_;
};

} // namespace type_graph
