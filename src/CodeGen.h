// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "ContainerInfo.h"
#include "OICodeGen.h"
#include "SymbolService.h"

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
  containerInfos_.reserve(containerConfigPaths.size());
  for (const auto &path : containerConfigPaths) {
    registerContainer(path);
  }
}

private:
  void registerContainer(const std::filesystem::path &path);

  type_graph::TypeGraph &typeGraph_;
  OICodeGen::Config config_;
  std::vector<ContainerInfo> containerInfos_;
  SymbolService& symbols_;
};
