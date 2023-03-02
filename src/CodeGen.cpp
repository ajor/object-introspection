#include "CodeGen.h"

#include <iostream> // TODO remove

#include "FuncGen.h"
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

  std::string code =
#include "OITraceCode.cpp"
      ;

  // TODO don't have this string here:
  code += "// storage macro definitions -----\n";
  if (true /* TODO: config.useDataSegment*/) {
    code += R"(
      #define SAVE_SIZE(val)
      #define SAVE_DATA(val)    StoreData(val, returnArg)

      #define JLOG(str)                           \
        do {                                      \
          if (__builtin_expect(logFile, 0)) {     \
            write(logFile, str, sizeof(str) - 1); \
          }                                       \
        } while (false)

      #define JLOGPTR(ptr)                    \
        do {                                  \
          if (__builtin_expect(logFile, 0)) { \
            __jlogptr((uintptr_t)ptr);        \
          }                                   \
        } while (false)
    )";
  } else {
    code += R"(
      #define SAVE_SIZE(val)    AddData(val, returnArg)
      #define SAVE_DATA(val)
      #define JLOG(str)
      #define JLOGPTR(ptr)
    )";
  }

  code += includes();

  // TODO comment about namespaces (copy from OICodeGen)
  code += "namespace OIInternal {\nnamespace {\n";
  FuncGen::DefineEncodeData(code);
  FuncGen::DefineEncodeDataSize(code);
  FuncGen::DefineStoreData(code);
  FuncGen::DefineAddData(code);
  // TODO don't have this string here:
  code += R"(
    template<typename T>
    void getSizeType(const T* s_ptr, size_t& returnArg)
    {
      JLOG("ptr val @");
      JLOGPTR(s_ptr);
      StoreData((uintptr_t)(s_ptr), returnArg);
      if (s_ptr && pointers.add((uintptr_t)s_ptr)) {
          getSizeType(*(s_ptr), returnArg);
      }
    }

    void getSizeType(const void *s_ptr, size_t& returnArg)
    {
      JLOG("void ptr @");
      JLOGPTR(s_ptr);
      StoreData((uintptr_t)(s_ptr), returnArg);
    }
  )";
  code += classDecls();
  code += classDefs();
  code += getSizeFuncs();
  code += "\nusing __ROOT_TYPE__ = " + rootType->name() + ";\n";
  code += "} // namespace\n} // namespace OIInternal\n";

  FuncGen::DefineTopLevelGetSizeRef(code, "");

  std::cout << code;
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
  std::string str = "void getSizeType(const " + c.name() + " &t, size_t &returnArg) {\n";
  for (const auto &member : c.members) {
    str += "  getSizeType(t." + member.name + ", returnArg);\n";
  }
  str += "}\n";
  return str;
}

std::string getContainerParams(const Container &c, bool typenamePrefix) {
  if (c.templateParams.empty())
    return "";

  std::string params = "<";
  for (size_t i=0; i<c.templateParams.size(); i++) {
    if (typenamePrefix)
      params += "typename ";
    params += "T" + std::to_string(i) + ",";
  }
  params.back() = '>';
  return params;
}

std::string CodeGen::getContainerSizeFunc(const Container &c) {
  std::string str;
  if (!c.templateParams.empty())
    str += "template " + getContainerParams(c, true) + "\n";
  str += "void getSizeType(const " + c.containerName() + getContainerParams(c, false) + " &container,";
  str += "size_t &returnArg) {";
  // TODO sort out templating + boilerplate
  str += c.containerInfo_.funcBody;
  str += "}\n";
  return str;
}

std::string CodeGen::includes() {
  std::string str;

  for (const auto t : typeGraph_.finalTypes) {
    if (const auto *c = dynamic_cast<Container*>(&t.get())) {
      str += "#include <" + c->containerInfo_.header + ">\n";
    }
  }

  return str;
}

std::string CodeGen::classDecls() {
  std::string decls;

//  for (const auto c : type_graph_.classes()) {
//    decls += decl(*c);
//  }
  for (const auto t : typeGraph_.finalTypes) {
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
  for (const auto t : typeGraph_.finalTypes) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      defs += def(*c);
  }

  return defs;
}

std::string CodeGen::getSizeFuncs() {
  std::string funcs;

  for (const auto t : typeGraph_.finalTypes) {
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
