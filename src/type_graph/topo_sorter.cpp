#include "topo_sorter.h"

namespace type_graph {

std::vector<Type*> TopoSorter::sort(const std::vector<Type*> &types) {
  for (auto type : types) {
    sort_type(type);
  }
  return sorted_types_;
}

void TopoSorter::sort_type(Type *type) {
  if (visited_.count(type) != 0)
    return;

  visited_.insert(type);
  type->accept(*this);
  sorted_types_.push_back(type);
}

void TopoSorter::visit(Class &c) {
  for (const auto &mem : c.members) {
    sort_type(mem.type);
  }
  for (const auto &parent : c.parents) {
    sort_type(parent.type);
  }
  for (const auto &template_param : c.template_params) {
    sort_type(template_param.type);
  }
}

void TopoSorter::visit(Container &c) {
  for (const auto &template_param : c.template_params) {
    sort_type(template_param.type);
  }
}

void TopoSorter::visit(Enum &e) {
}

void TopoSorter::visit(Primitive &p) {
}

void TopoSorter::visit(Typedef &td) {
  // TODO
}

void TopoSorter::visit(Pointer &p) {
  // TODO what are references defined as?
}

void TopoSorter::visit(Array &a) {
  sort_type(a.element_type());
}

} // namespace type_graph
