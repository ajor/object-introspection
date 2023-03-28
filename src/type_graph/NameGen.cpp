#include "NameGen.h"

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

Pass NameGen::createPass() {
  auto fn = [](TypeGraph &typeGraph) {
    NameGen nameGen;
    nameGen.generateNames(typeGraph.rootTypes());
  };

  return Pass("NameGen", fn);
}

void NameGen::generateNames(const std::vector<ref<Type>> &types) {
  for (auto &type : types) {
    visit(type);
  }
};

void NameGen::visit(Type &type) {
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
  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
  for (const auto &parent : c.parents) {
    visit(*parent.type);
  }
  for (const auto &member : c.members) {
    visit(*member.type);
  }

  std::string name = c.name();
  removeTemplateParams(name);

  name.push_back('_');
  for (const auto &param : c.templateParams) {
    name += param.type->name();
    name.push_back('_');
  }

  // TODO deduplicate types of members
  // TODO deduplicate member names (can happen after flattening)

  // Append an incrementing number to ensure we don't get duplicates
  name += std::to_string(n++);

  c.setName(name);
}

void NameGen::visit(Container &c) {
  for (const auto &template_param : c.templateParams) {
    visit(*template_param.type);
  }

  std::string name = c.name();
  removeTemplateParams(name);

  name.push_back('<');
  for (const auto &param : c.templateParams) {
    name += param.type->name();
    name += ", ";
  }
  name.pop_back();
  name.pop_back();
  name.push_back('>');

  c.setName(name);
}

void NameGen::visit(Array &a) {
  visit(*a.elementType());
}

void NameGen::visit(Typedef &td) {
  visit(*td.underlyingType());
}

void NameGen::visit(Pointer &p) {
  visit(*p.pointeeType());
}

void NameGen::visit(DummyAllocator &d) {
  visit(d.allocType());
}

} // namespace type_graph
