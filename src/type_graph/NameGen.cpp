#include "NameGen.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

void NameGen::generateNames(const std::vector<ref<Type>> &types) {
  for (auto &type : types) {
    nameType(type);
  }
};

void NameGen::nameType(Type &type) {
  if (visited_.count(&type) != 0)
    return;

  visited_.insert(&type);
  type.accept(*this);
}

/*
 * Remove template parameters from the type name
 *
 * "std::vector<int>" -> "std::vector"
 */
void NameGen::removeTemplateParams(std::string &name) {
  auto template_start_pos = name.find('<');
  if (template_start_pos != std::string::npos)
    name.erase(template_start_pos);
}

void NameGen::visit(Class &c) {
  for (const auto &template_param : c.templateParams) {
    nameType(*template_param.type);
  }

  std::string name = c.name();
  removeTemplateParams(name);

  name.push_back('_');
  for (const auto &param : c.templateParams) {
    name += param.type->name();
    name.push_back('_');
  }

  // Append an incrementing number to ensure we don't get duplicates
  name += std::to_string(n++);

  c.setName(name);
}

void NameGen::visit(Container &c) {
  for (const auto &template_param : c.templateParams) {
    nameType(*template_param.type);
  }

  std::string name = c.name();
  removeTemplateParams(name);

  name.push_back('<');
  // TODO check which container params should be kept
  for (const auto &param : c.templateParams) {
    name += param.type->name();
    name += ", ";
  }
  name.pop_back();
  name.pop_back();
  name.push_back('>');

  c.setName(name);
}

void NameGen::visit(Enum &e) {
}

void NameGen::visit(Primitive &p) {
}

void NameGen::visit(Typedef &td) {
}

void NameGen::visit(Pointer &p) {
}

void NameGen::visit(Array &a) {
}

} // namespace type_graph
