#include "TopoSorter.h"

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

Pass TopoSorter::createPass() {
  auto fn = [](TypeGraph &typeGraph) {
    TopoSorter sorter;
    sorter.sort(typeGraph.rootTypes());
    typeGraph.finalTypes = sorter.sortedTypes();
  };

  return Pass("TopoSorter", fn);
}

void TopoSorter::sort(const std::vector<ref<Type>> &types) {
  for (const auto &type : types) {
    typesToSort_.push(type);
  }
  while (!typesToSort_.empty()) {
    visit(typesToSort_.front());
    typesToSort_.pop();
  }
}

const std::vector<ref<Type>> &TopoSorter::sortedTypes() const {
  return sortedTypes_;
}

void TopoSorter::visit(Type &type) {
  if (visited_.count(&type) != 0)
    return;

  visited_.insert(&type);
  type.accept(*this);
  sortedTypes_.push_back(type);
}

void TopoSorter::visit(Pointer &p) {
  // Pointers do not create a dependency, but we do still care about the types
  // they point to, so delay them until the end.
  typesToSort_.push(*p.pointeeType());
}

} // namespace type_graph
