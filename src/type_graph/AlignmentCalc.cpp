#include "AlignmentCalc.h"

#include <cassert>

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

void AlignmentCalc::calculateAlignments(const std::vector<ref<Type>> &types) {
  for (auto &type : types) {
    calculateAlignment(type);
  }
};

void AlignmentCalc::calculateAlignment(Type &type) {
  type.accept(*this);
}

void AlignmentCalc::visit(Class &c) {
  uint64_t alignment = 1;
  for (auto &member : c.members) {
    if (member.align == 0) {
      // If the member does not have an explicit alignment, calculate it from
      // the member's type.
      calculateAlignment(*member.type);
      member.align = member.type->align();
    }
    alignment = std::max(alignment, member.align);
  }

  // AlignmentCalc should be run after Flattener
  assert(c.parents.empty());

  c.setAlign(alignment);
}

void AlignmentCalc::visit(Container &c) {
}

void AlignmentCalc::visit(Enum &e) {
}

void AlignmentCalc::visit(Primitive &p) {
}

void AlignmentCalc::visit(Typedef &td) {
}

void AlignmentCalc::visit(Pointer &p) {
}

void AlignmentCalc::visit(Array &a) {
}

} // namespace type_graph
