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

  bool generate(drgn_type *drgnType, std::string& code);
  void loadConfig(const std::set<std::filesystem::path> &containerConfigPaths);

private:
  void registerContainer(const std::filesystem::path &path);

  type_graph::TypeGraph &typeGraph_;
  OICodeGen::Config config_;
  std::vector<ContainerInfo> containerInfos_;
  SymbolService& symbols_;
};
