// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "ContainerInfo.h"

#include "OICodeGen.h" // For OICodeGen::Config

namespace fs = std::filesystem;

struct drgn_type;

namespace type_graph {
class Container;
class Type;
class TypeGraph;
} // namespace type_graph

class CodeGen {
public:
  CodeGen(type_graph::TypeGraph &typeGraph, OICodeGen::Config &config) : typeGraph_(typeGraph), config_(config) { }
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
  std::string classDecls();
  std::string classDefs();
  std::string getSizeFuncDecls();
  std::string getSizeFuncDefs();
  void registerContainer(const fs::path &path);
  std::string getContainerSizeFuncDecl(const type_graph::Container &c);
  std::string getContainerSizeFuncDef(const type_graph::Container &c);

  type_graph::TypeGraph &typeGraph_;
  OICodeGen::Config config_;
  std::vector<ContainerInfo> containerInfos_;
};
