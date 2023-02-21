#include "NameGen.h"

namespace type_graph {

void NameGen::generateNames(const std::vector<Type*> &types) {
  for (auto *type : types) {
    type->accept(*this);
  }
};

// TODO no need to be a visitor?
void NameGen::visit(Class &c) {
  // Remove template parameters from the type name
  std::string name = c.name();
  auto template_start_pos = name.find('<');
  if (template_start_pos != std::string::npos)
    name.erase(template_start_pos);

  name.push_back('_');
  for (const auto &param : c.template_params) {
    name += param.type->name();
    name.push_back('_');
  }

  // Append an incrementing number to ensure we don't get duplicates
  name += std::to_string(n++);

  c.setName(name);

//    std::string str = name_;
//    if (!template_params.empty()) {
//      str.push_back('_');
//      for (const auto &tparam : template_params) {
//        // TODO has value?
//        str += tparam.type->name() + "_";
//      }
//      // todo nasty
//      str.pop_back();
//      str.push_back('_');
//    }
//    return str;
}

void NameGen::visit(Container &c) {
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
