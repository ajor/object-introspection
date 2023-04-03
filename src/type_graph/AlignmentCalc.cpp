#include "AlignmentCalc.h"

#include <cassert>

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

Pass AlignmentCalc::createPass() {
  auto fn = [](TypeGraph &typeGraph) {
    AlignmentCalc alignmentCalc;
    alignmentCalc.calculateAlignments(typeGraph.rootTypes());
  };

  return Pass("AlignmentCalc", fn);
}

void AlignmentCalc::calculateAlignments(const std::vector<ref<Type>> &types) {
  for (auto &type : types) {
    visit(type);
  }
};

void AlignmentCalc::visit(Type &type) {
  if (visited_.count(&type) != 0)
    return;

  visited_.insert(&type);
  type.accept(*this);
}

void AlignmentCalc::visit(Class &c) {
  uint64_t alignment = 1;
  for (auto &member : c.members) {
    if (member.align == 0) {
      // If the member does not have an explicit alignment, calculate it from
      // the member's type.
      visit(*member.type);
      member.align = member.type->align();
    }
    alignment = std::max(alignment, member.align);
  }

  // AlignmentCalc should be run after Flattener
  assert(c.parents.empty());

  c.setAlign(alignment);
}

} // namespace type_graph
