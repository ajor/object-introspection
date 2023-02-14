// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <string>

#include "ContainerInfo.h"
#include "type_graph/type_graph.h"

namespace fs = std::filesystem;

class OICodeGen2 {
public:
  OICodeGen2(const type_graph::TypeGraph &type_graph) : type_graph_(type_graph) { }
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

// TODO private:
  std::vector<ContainerInfo> containerInfos;

private:
  void registerContainer(const fs::path &path);

  const type_graph::TypeGraph &type_graph_;
};
