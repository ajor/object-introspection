// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "ContainerInfo.h"
#include "OICodeGen.h"
#include "SymbolService.h"

namespace fs = std::filesystem;

struct drgn_type;

namespace type_graph {
class Class;
class Container;
class Type;
class TypeGraph;
} // namespace type_graph

class CodeGen {
public:
  CodeGen(type_graph::TypeGraph &typeGraph, OICodeGen::Config &config, SymbolService& symbols) : typeGraph_(typeGraph), config_(config), symbols_(symbols) { }
  std::string generate(drgn_type *drgnType);

// TODO shouldn't need to be a template (but shouldn't be a set!)
template <typename T>
void loadConfig(const T &containerConfigPaths) {
  for (const auto &path : containerConfigPaths) {
    registerContainer(path);
  }
}

private:
  std::string includes();
  std::string getSizeFuncDecls();
  std::string getSizeFuncDefs();
  std::string getClassSizeFuncDef(const type_graph::Class &c);
  void registerContainer(const fs::path &path);
  std::string getContainerSizeFuncDecl(const type_graph::Container &c);
  std::string getContainerSizeFuncDef(const type_graph::Container &c);

  type_graph::TypeGraph &typeGraph_;
  OICodeGen::Config config_;
  std::vector<ContainerInfo> containerInfos_;
  SymbolService& symbols_;
};
