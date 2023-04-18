#include "TopoSorter.h"

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

Pass TopoSorter::createPass() {
  auto fn = [](TypeGraph &typeGraph) {
    TopoSorter sorter;
    sorter.sort(typeGraph.rootTypes());
    typeGraph.finalTypes = std::move(sorter.sortedTypes());
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
}

void TopoSorter::visit(Class &c) {
  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
  for (const auto &parent : c.parents) {
    visit(*parent.type);
  }
  for (const auto &mem : c.members) {
    visit(*mem.type);
  }
  sortedTypes_.push_back(c);

  // Same as pointers, child do not create a dependency so are delayed until the end
  for (const auto &child : c.children) {
    typesToSort_.push(child);
  }
}

void TopoSorter::visit(Container &c) {
  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
  sortedTypes_.push_back(c);
}

void TopoSorter::visit(Enum &e) {
  sortedTypes_.push_back(e);
}

void TopoSorter::visit(Typedef &td) {
  visit(*td.underlyingType());
  sortedTypes_.push_back(td);
}

void TopoSorter::visit(Pointer &p) {
  // Pointers do not create a dependency, but we do still care about the types
  // they point to, so delay them until the end.
  typesToSort_.push(*p.pointeeType());
}

} // namespace type_graph
