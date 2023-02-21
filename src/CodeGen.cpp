#include "CodeGen.h"

#include <iostream> // TODO remove

#include "type_graph/Flattener.h"
#include "type_graph/NameGen.h"
#include "type_graph/RequiredTypeCollector.h"
#include "type_graph/TopoSorter.h"
#include "type_graph/TypeGraph.h"

using namespace type_graph;

void CodeGen::generate(Type &rootType) {
  // TODO free resources from visitor classes after running each one
  RequiredTypeCollector req_types;
  auto required_types = req_types.collect({&rootType});
  // TODO try Flattener.flatten()
  Flattener flattener;
  flattener.flatten(req_types.classes());
  TopoSorter topo_sort;
  auto sorted_types = topo_sort.sort(required_types);

  NameGen nameGen;
  nameGen.generateNames(sorted_types);

  std::cout << "sorted types:\n";
  for (auto &t : sorted_types) {
    std::cout << t->name() << std::endl;
  };

  std::cout << "class decls\n";
  std::cout << ClassDecls(sorted_types) << std::endl;
  std::cout << "class defs\n";
  std::cout << ClassDefs(sorted_types) << std::endl;
  std::cout << "get size funcs:\n";
  std::cout << GetSizeFuncs(sorted_types) << std::endl;
}

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

std::string CodeGen::getContainerSizeFunc(const Container &c) {
  std::string str = "void getSize(const " + c.name() + " &t, size_t &size) {\n";
  // TODO pick the right container
  // TODO sort out templating + boilerplate
  str += containerInfos[0].funcBody;
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

std::string CodeGen::ClassDecls(const std::vector<Type*>& types) {
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

std::string CodeGen::ClassDefs(const std::vector<Type*>& types) {
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

std::string CodeGen::GetSizeFuncs(const std::vector<Type*>& types) {
  std::string funcs;

  for (const auto t : types) {
    if (const auto *c = dynamic_cast<Class*>(t))
      funcs += getClassSizeFunc(*c);
    if (const auto *c = dynamic_cast<Container*>(t))
      funcs += getContainerSizeFunc(*c);
  }

  return funcs;
}

void CodeGen::registerContainer(const fs::path &path) {
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
