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
  std::string name = c.name();
  removeTemplateParams(name);

  // Append an incrementing number to ensure we don't get duplicates
  c.setName(name + "_" + std::to_string(n++));

  // Deduplicate member names. Duplicates may be present after flattening.
  for (size_t i=0; i<c.members.size(); i++) {
    c.members[i].name += "_" + std::to_string(i);
  }


  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
  for (const auto &parent : c.parents) {
    visit(*parent.type);
  }
  for (const auto &member : c.members) {
    visit(*member.type);
  }
}

void NameGen::visit(Container &c) {
  for (const auto &template_param : c.templateParams) {
    visit(*template_param.type);
  }

  std::string name = c.name();
  removeTemplateParams(name);

  name.push_back('<');
  for (const auto &param : c.templateParams) {
    if (param.value) {
      name += *param.value;
    }
    else {
      name += param.type->name();
    }
    name += ", ";
  }
  name.pop_back();
  name.pop_back();
  name.push_back('>');

  c.setName(name);
}

} // namespace type_graph
