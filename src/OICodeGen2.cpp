#include "OICodeGen2.h"

// TODO use C++20 std::format?
std::string decl(const Class &c) {
  return "struct " + c.name() + ";\n";
}

std::string def(const Class &c) {
  // TODO use Class.kind_
  std::string str = "struct " + c.name() + " {\n";
  for (const auto &mem : c.members) {
    str += "  " + mem.type->name() + " " + mem.name + ";\n";
  }
  str += "};\n";
  return str;
}

//std::string name(const Container &c) {
//  std::string str = c.name();
//  if (!c.template_params_.empty()) {
//    str.push_back('<');
//    for (const auto &tparam : c.template_params_) {
//      // TODO has value?
//      str += tparam.type->name() + ", ";
//    }
//    // TODO nasty
//    str.pop_back();
//    str.pop_back();
//    str.push_back('>');
//  }
//  return str;
//}

std::string OICodeGen2::ClassDecls() {
  std::string decls;

  for (const auto c : type_graph_.classes()) {
    decls += decl(*c);
  }

  return decls;
}

std::string OICodeGen2::ClassDefs() {
  std::string decls;

  for (const auto c : type_graph_.classes()) {
    decls += def(*c);
  }

  return decls;
}
