#include "CodeGen.h"

#include <iostream> // TODO remove

#include <boost/format.hpp>

#include "FuncGen.h"
// TODO put passes into their own directory/namespace
#include "type_graph/AddChildren.h"
#include "type_graph/AddPadding.h"
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

namespace {
void genStaticAssertsClass(const Class& c, std::string& code) {
  code += "static_assert(sizeof(" + c.name() + ") == " + std::to_string(c.size()) + ", \"Unexpected size of struct " + c.name() + "\");\n";
  for (const auto &member : c.members) {
    code += "static_assert(offsetof(" + c.name() + ", " + member.name + ") == " + std::to_string(member.offset) + ", \"Unexpected offset of " + c.name() + "::" + member.name + "\");\n";
  }
  code.push_back('\n');
}

void genStaticAssertsContainer(const Container& c, std::string& code) {
  code += "static_assert(sizeof(" + c.name() + ") == " + std::to_string(c.size()) + ", \"Unexpected size of container " + c.name() + "\");\n";
  code.push_back('\n');
}

void genStaticAsserts(const TypeGraph& typeGraph, std::string& code) {
  for (const auto t : typeGraph.finalTypes) {
    if (const auto *c = dynamic_cast<Class*>(&t.get()))
      genStaticAssertsClass(*c, code);
    if (const auto *c = dynamic_cast<Container*>(&t.get()))
      genStaticAssertsContainer(*c, code);
  }
}
} // namespace

std::string CodeGen::generate(drgn_type *drgnType) {
  // TODO wrap in try-catch
  // This scope is unrealted to the above comment - it is to avoid parsedRoot being available elsewhere
  // because typeGraph.rootTypes() should be used instead, in case the root types have been modified
  DrgnParser drgnParser(typeGraph_, containerInfos_, config_.chaseRawPointers);
  {
    Type *parsedRoot = drgnParser.parse(drgnType);
    typeGraph_.addRoot(*parsedRoot);
  }

  PassManager pm;
  pm.addPass(Flattener::createPass());
  pm.addPass(TypeIdentifier::createPass(containerInfos_));
  if (config_.polymorphicInheritance) {
    pm.addPass(AddChildren::createPass(drgnParser, symbols_));
    // Re-run passes over newly added children
    pm.addPass(Flattener::createPass());
    pm.addPass(TypeIdentifier::createPass(containerInfos_));
  }
  pm.addPass(AddPadding::createPass());
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
  genStaticAsserts(typeGraph_, code);


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
  std::string str = "void getSizeType(const " + c.name() + " &t, size_t &returnArg);\n";
  return str;
}

std::string CodeGen::getClassSizeFuncDef(const Class &c) {
  // TODO check config.polymorphicInheritance setting
  std::string funcName = "getSizeType";
  if (c.isDynamic()) {
    funcName = "getSizeTypeConcrete";
  }

  std::string str = "void " + funcName + "(const " + c.name() + " &t, size_t &returnArg) {\n";
  for (const auto &member : c.members) {
    str += "  JLOG(\"" + member.name + " @\");\n";
    str += "  JLOGPTR(&t." + member.name + ");\n";
    str += "  getSizeType(t." + member.name + ", returnArg);\n";
  }
  str += "}\n";

  if (c.isDynamic()) {
    std::vector<SymbolInfo> childVtableAddrs;
    childVtableAddrs.reserve(c.children.size());

    for (const Type& childType : c.children) {
      auto *childClass = dynamic_cast<const Class*>(&childType);
      if (childClass == nullptr) {
        abort(); // TODO
      }
      //      TODO:
//      auto fqChildName = *fullyQualifiedName(child);
      auto fqChildName = "TODO - implement me";

      // We must split this assignment and append because the C++ standard lacks
      // an operator for concatenating std::string and std::string_view...
      std::string childVtableName = "vtable for ";
      childVtableName += fqChildName;

      auto optVtableSym = symbols_.locateSymbol(childVtableName, true);
      if (!optVtableSym) {
//        LOG(ERROR) << "Failed to find vtable address for '" << childVtableName;
//        LOG(ERROR) << "Falling back to non dynamic mode";
        childVtableAddrs.clear(); // TODO why??
        break;
      }
      childVtableAddrs.push_back(*optVtableSym);
    }

    str += "void getSizeType(const " + c.name() + " &t, size_t &returnArg) {\n";
    str += "  auto *vptr = *reinterpret_cast<uintptr_t * const *>(&t);\n";
    str += "  uintptr_t topOffset = *(vptr - 2);\n";
    str += "  uintptr_t vptrVal = reinterpret_cast<uintptr_t>(vptr);\n";

    for (size_t i = 0; i < c.children.size(); i++) {
      // The vptr will point to *somewhere* in the vtable of this object's
      // concrete class. The exact offset into the vtable can vary based on a
      // number of factors, so we compare the vptr against the vtable range for
      // each possible class to determine the concrete type.
      //
      // This works for C++ compilers which follow the GNU v3 ABI, i.e. GCC and
      // Clang. Other compilers may differ.
      const Type& child = c.children[i];
      auto& vtableSym = childVtableAddrs[i];
      uintptr_t vtableMinAddr = vtableSym.addr;
      uintptr_t vtableMaxAddr = vtableSym.addr + vtableSym.size;
      str += "  if (vptrVal >= 0x" +
              (boost::format("%x") % vtableMinAddr).str() + " && vptrVal < 0x" +
              (boost::format("%x") % vtableMaxAddr).str() + ") {\n";
      str += "    SAVE_DATA(" + std::to_string(i) + ");\n";
      str +=
          "    uintptr_t baseAddress = reinterpret_cast<uintptr_t>(&t) + "
          "topOffset;\n";
      str += "    getSizeTypeConcrete(*reinterpret_cast<const " + child.name() +
              "*>(baseAddress), returnArg);\n";
      str += "    return;\n";
      str += "  }\n";
    }

    str += "  SAVE_DATA(-1);\n";
    str += "  getSizeTypeConcrete(t, returnArg);\n";
    str += "}\n";
  }

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
