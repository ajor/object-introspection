#include "CodeGen.h"

#include <iostream> // TODO remove

#include "type_graph/AlignmentCalc.h"
#include "type_graph/DrgnParser.h"
#include "type_graph/Flattener.h"
#include "type_graph/NameGen.h"
#include "type_graph/TopoSorter.h"
#include "type_graph/TypeGraph.h"
#include "type_graph/Types.h"

// TODO don't do this:
using namespace type_graph;

template <typename T>
using ref = std::reference_wrapper<T>;

std::string CodeGen::generate(drgn_type *drgnType) {
  // TODO wrap in try-catch
  DrgnParser drgnParser(typeGraph_, containerInfos_);
  Type *rootType = drgnParser.parse(drgnType);
  typeGraph_.addRoot(*rootType);

  PassManager pm;
  pm.addPass(Flattener::createPass());
  pm.addPass(NameGen::createPass());
  pm.addPass(AlignmentCalc::createPass());
  pm.addPass(TopoSorter::createPass());
  pm.run(typeGraph_, true); // TODO don't alway run with debug

  std::cout << "sorted types:\n";
  for (auto &t : typeGraph_.finalTypes) {
    std::cout << t.get().name() << std::endl;
  };

  std::string code;
  code += includes();
  code += classDecls();
  code += classDefs();
  code += getSizeFuncs();
  return code;
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
  str += containerInfos_[0].funcBody;
  str += "}\n";
  return str;
}

std::string CodeGen::includes() {
  std::string str;

  for (const auto t : typeGraph_.finalTypes()) {
    if (const auto *c = dynamic_cast<Container*>(&t.get())) {
      // TODO pick the right container
      str += "#include <" + containerInfos_[0].header + ">\n";
    }
  }

  return str;
}

std::string CodeGen::classDecls() {
  std::string decls;

//  for (const auto c : type_graph_.classes()) {
//    decls += decl(*c);
//  }
  for (const auto t : typeGraph_.finalTypes()) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      decls += decl(*c);
  }

  return decls;
}

std::string CodeGen::classDefs() {
  std::string defs;

//  for (const auto c : type_graph_.classes()) {
//    defs += def(*c);
//  }
  for (const auto t : typeGraph_.finalTypes()) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      defs += def(*c);
  }

  return defs;
}

std::string CodeGen::getSizeFuncs() {
  std::string funcs;

  for (const auto t : typeGraph_.finalTypes()) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      funcs += getClassSizeFunc(*c);
    if (const auto *c = dynamic_cast<Container*>(&t.get()))
      funcs += getContainerSizeFunc(*c);
  }

  return funcs;
}

void CodeGen::registerContainer(const fs::path &path) {
// TODO don't catch exceptions at this level?
//  try {
  const auto &info = containerInfos_.emplace_back(path);
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
