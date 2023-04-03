#include "CodeGen.h"

#include <iostream> // TODO remove

#include <boost/format.hpp>

#include "FuncGen.h"
#include "SymbolService.h"
// TODO put passes into their own directory/namespace
#include "type_graph/AlignmentCalc.h"
#include "type_graph/DrgnParser.h"
#include "type_graph/Flattener.h"
#include "type_graph/GenDecls.h"
#include "type_graph/GenDefs.h"
#include "type_graph/NameGen.h"
#include "type_graph/RemoveTopLevelPointer.h"
#include "type_graph/TopoSorter.h"
#include "type_graph/TypeGraph.h"
#include "type_graph/TypeIdentifier.h"
#include "type_graph/Types.h"

// TODO don't do this:
using namespace type_graph;

template <typename T>
using ref = std::reference_wrapper<T>;

std::string CodeGen::generate(drgn_type *drgnType) {
  // TODO wrap in try-catch
  // This scope is unrealted to the above comment - it is to avoid parsedRoot being available elsewhere
  // because typeGraph.rootTypes() should be used instead, in case the root types have been modified
  {
    DrgnParser drgnParser(typeGraph_, containerInfos_, config_.chaseRawPointers);
    Type *parsedRoot = drgnParser.parse(drgnType);
    typeGraph_.addRoot(*parsedRoot);
  }

  PassManager pm;
  pm.addPass(Flattener::createPass());
  pm.addPass(TypeIdentifier::createPass(containerInfos_));
  pm.addPass(NameGen::createPass());
  pm.addPass(AlignmentCalc::createPass());
  pm.addPass(RemoveTopLevelPointer::createPass());
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

      template<typename T, int N>
      struct OIArray {
        T vals[N];
      };
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
  FuncGen::DeclareGetContainer(code);
  GenDecls::run(typeGraph_, code);
  GenDefs::run(typeGraph_, code);


  code += R"(
    template <typename T>
    void getSizeType(const T &t, size_t& returnArg);

    template<typename T>
    void getSizeType(/*const*/ T* s_ptr, size_t& returnArg);

    void getSizeType(/*const*/ void *s_ptr, size_t& returnArg);

    template <typename T, int N>
    void getSizeType(const OIArray<T,N>& container, size_t& returnArg);
  )";

  code += getSizeFuncDecls();
  // TODO don't have this string here:
  // TODO use macros, not StoreData directly
  code += R"(
    template <typename T>
    void getSizeType(const T &t, size_t& returnArg) {
      JLOG("obj @");
      JLOGPTR(&t);
      SAVE_SIZE(sizeof(T));
    }
  )";
  // TODO const and non-const versions
  // OR maybe just remove const everywhere
  code += R"(
    template<typename T>
    void getSizeType(/*const*/ T* s_ptr, size_t& returnArg)
    {
      JLOG("ptr val @");
      JLOGPTR(s_ptr);
      StoreData((uintptr_t)(s_ptr), returnArg);
      if (s_ptr && pointers.add((uintptr_t)s_ptr)) {
          getSizeType(*(s_ptr), returnArg);
      }
    }

    void getSizeType(/*const*/ void *s_ptr, size_t& returnArg)
    {
      JLOG("void ptr @");
      JLOGPTR(s_ptr);
      StoreData((uintptr_t)(s_ptr), returnArg);
    }

    template <typename T, int N>
    void getSizeType(const OIArray<T,N>& container, size_t& returnArg)
    {
      SAVE_DATA((uintptr_t)N);
      SAVE_SIZE(sizeof(container));

      for (size_t i=0; i<N; i++) {
          // undo the static size that has already been added per-element
          SAVE_SIZE(-sizeof(container.vals[i]));
          getSizeType(container.vals[i], returnArg);
      }
    }
  )";

  code += getSizeFuncDefs();

  assert(typeGraph_.rootTypes().size() == 1);
  Type &rootType = typeGraph_.rootTypes()[0];
  code += "\nusing __ROOT_TYPE__ = " + rootType.name() + ";\n";
  code += "} // namespace\n} // namespace OIInternal\n";

  FuncGen::DefineTopLevelGetSizeRef(code, SymbolService::getTypeName(drgnType));

  std::cout << code;
  return code;
}

std::string getClassSizeFuncDecl(const Class &c) {
  // TODO don't duplicate code from getClassSizeFuncDef
  std::string str = "void getSizeType(const " + c.name() + " &t, size_t &returnArg);\n";
  return str;
}

std::string getClassSizeFuncDef(const Class &c) {
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

std::string CodeGen::getContainerSizeFuncDecl(const Container &c) {
  auto fmt = boost::format(c.containerInfo_.funcDecl) % c.containerInfo_.typeName;
  return fmt.str();

  // TODO don't duplicate logic with getContainerSizeFuncDef
//  std::string str;
//  if (!c.templateParams.empty())
//    str += "template " + getContainerParams(c, true) + "\n";
//  str += "void getSizeType(const " + c.containerName() + getContainerParams(c, false) + " &container,";
//  str += "size_t &returnArg);\n";
//  return str;
}

std::string CodeGen::getContainerSizeFuncDef(const Container &c) {
  // TODO this set is a nasty hack:
  static std::unordered_set<ContainerTypeEnum> usedContainers{};
  if (usedContainers.find(c.containerInfo_.ctype) != usedContainers.end()) {
    return "";
  }
  usedContainers.insert(c.containerInfo_.ctype);

  auto fmt = boost::format(c.containerInfo_.funcBody) % c.containerInfo_.typeName;
  return fmt.str();

//  std::string str;
//  if (!c.templateParams.empty())
//    str += "template " + getContainerParams(c, true) + "\n";
//  str += "void getSizeType(const " + c.containerName() + getContainerParams(c, false) + " &container,";
//  str += "size_t &returnArg) {\n";
//  // TODO sort out templating + boilerplate
//  str += c.containerInfo_.funcBody;
//  str += "}\n";
//  return str;
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

std::string CodeGen::getSizeFuncDecls() {
  std::string funcs;

  for (const auto t : typeGraph_.finalTypes) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      funcs += getClassSizeFuncDecl(*c);
    if (const auto *c = dynamic_cast<Container*>(&t.get()))
      funcs += getContainerSizeFuncDecl(*c);
  }

  return funcs;
}

std::string CodeGen::getSizeFuncDefs() {
  std::string funcs;

  for (const auto t : typeGraph_.finalTypes) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      funcs += getClassSizeFuncDef(*c);
    if (const auto *c = dynamic_cast<Container*>(&t.get()))
      funcs += getContainerSizeFuncDef(*c);
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
