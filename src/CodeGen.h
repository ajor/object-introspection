// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <string>

#include "ContainerInfo.h"
#include "type_graph/TypeGraph.h"

namespace fs = std::filesystem;

class CodeGen {
public:
  CodeGen(type_graph::TypeGraph &typeGraph) : typeGraph_(typeGraph) { }
  void generate(type_graph::Type &rootType);
  std::string ClassDecls(const std::vector<type_graph::Type*>& types);
  std::string ClassDefs(const std::vector<type_graph::Type*>& types);
  std::string GetSizeFuncs(const std::vector<type_graph::Type*>& types);

// TODO shouldn't need to be a template (but shouldn't be a set!)
template <typename T>
void loadConfig(const T &containerConfigPaths) {
  for (const auto &path : containerConfigPaths) {
    registerContainer(path);
  }
}

  std::vector<ContainerInfo> containerInfos;

private:
  void registerContainer(const fs::path &path);
  std::string getContainerSizeFunc(const type_graph::Container &c);

  type_graph::TypeGraph &typeGraph_;
};
