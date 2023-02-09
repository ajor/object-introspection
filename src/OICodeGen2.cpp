#include "OICodeGen2.h"

using namespace type_graph;

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

std::string func(const Class &c) {
  std::string str = "void getSize(const " + c.name() + "& t, blah result) {\n";
  for (const auto &member : c.members) {
    str += "  getSize(t." + member.name + ", result);\n";
  }
  str += "}\n";
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

std::string OICodeGen2::ClassDecls(const std::vector<Type*>& types) {
  std::string decls;

//  for (const auto c : type_graph_.classes()) {
//    decls += decl(*c);
//  }
  for (const auto t : types) {
    if (const auto *c = dynamic_cast<Class*>(t))
      decls += decl(*c);
  }

  return decls;
}

std::string OICodeGen2::ClassDefs(const std::vector<Type*>& types) {
  std::string defs;

//  for (const auto c : type_graph_.classes()) {
//    defs += def(*c);
//  }
  for (const auto t : types) {
    if (const auto *c = dynamic_cast<Class*>(t))
      defs += def(*c);
  }

  return defs;
}

std::string OICodeGen2::GetSizeFuncs(const std::vector<Type*>& types) {
  std::string funcs;

  for (const auto t : types) {
    if (const auto *c = dynamic_cast<Class*>(t))
      funcs += func(*c);
  }

  return funcs;
}
