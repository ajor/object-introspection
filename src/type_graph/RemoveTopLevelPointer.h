#pragma once

#include <functional>
#include <vector>

#include "PassManager.h"
#include "Types.h"
#include "Visitor.h"

namespace type_graph {

/*
 * RemoveTopLevelPointer
 *
 * If the top type node is a pointer, remove it from the graph and instead have
 * the pointee type as the top-level node.
 */
class RemoveTopLevelPointer : public LazyVisitor {
public:
  static Pass createPass();

  void removeTopLevelPointers(std::vector<std::reference_wrapper<Type>> &types);
  void visit(Pointer &p) override;

private:
  Type *topLevelType_ = nullptr;
};

} // namespace type_graph
