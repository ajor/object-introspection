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

std::string getClassSizeFunc(const Class &c) {
  std::string str = "void getSize(const " + c.name() + " &t, size_t &size) {\n";
  for (const auto &member : c.members) {
    str += "  getSize(t." + member.name + ", result);\n";
  }
  str += "}\n";
  return str;
}

std::string getContainerSizeFunc(const Container &c) {
  std::string str = "void getSize(const " + c.name() + " &t, size_t &size) {\n";
  str += "  TODO - get code from TOML files\n";
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
      funcs += getClassSizeFunc(*c);
    if (const auto *c = dynamic_cast<Container*>(t))
      funcs += getContainerSizeFunc(*c);
  }

  return funcs;
}

void OICodeGen2::registerContainer(const fs::path &path) {
// TODO don't catch exceptions at this level?
//  try {
  const auto &info = containerInfos.emplace_back(path);
//  }
//  catch (const toml::parse_error &err) {
//    // TODO
//  }
//  // TODO more exception types

  // TODO replace this function:
  // TODO rewrite FuncGen:
  //   - use free functions
  //   - TOML should not need to define declarations or function names - only function contents
//  if (!funcGen.RegisterContainer(info.ctype, path)) {
//    // TODO throw
//    throw std::runtime_error("bad funcgen container");
//  }

//  VLOG(1) << "registered container: " << info.typeName;
}
