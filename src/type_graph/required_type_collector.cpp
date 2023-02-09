#include "required_type_collector.h"

namespace type_graph {

std::vector<Type*> RequiredTypeCollector::collect(const std::vector<Type*> &types) {
  for (auto type : types) {
    depth = 0;
    collect_type(type);
  }
  return required_types_;
}

void RequiredTypeCollector::collect_type(Type *type) {
  if (visited_.count(type) != 0)
    return;

  visited_.insert(type);
  depth++;
  type->accept(*this);
  depth--;
}

void RequiredTypeCollector::visit(Class &c) {
  required_types_.push_back(&c);
  required_classes_.push_back(&c);
  for (const auto &mem : c.members) {
    collect_type(mem.type);
  }
  // Parents and template parameters are not required as they will be flattened
  // into derived types.
}

void RequiredTypeCollector::visit(Container &c) {
  // Containers themselves don't need to be collected, but their template
  // parameters might be required
  for (const auto &template_param : c.template_params) {
    collect_type(template_param.type);
  }
}

void RequiredTypeCollector::visit(Enum &e) {
  required_types_.push_back(&e);
}

void RequiredTypeCollector::visit(Primitive &p) {
}

void RequiredTypeCollector::visit(TypeDef &td) {
  required_types_.push_back(&td);
}

// TODO chase raw pointers flag
void RequiredTypeCollector::visit(Pointer &p) {
  if (chasePointer()) {
    collect_type(p.pointee_type());
  }
}

void RequiredTypeCollector::visit(Array &a) {
  collect_type(a.element_type());
}

bool RequiredTypeCollector::chasePointer() const {
  // Chase top-level pointers
  return depth == 1;
}

} // namespace type_graph
