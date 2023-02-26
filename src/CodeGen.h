// TODO copyright notices everywhere
#pragma once

#include <filesystem>
#include <functional>
#include <string>

#include "ContainerInfo.h"

namespace fs = std::filesystem;

struct drgn_type;

namespace type_graph {
class Container;
class Type;
class TypeGraph;
} // namespace type_graph

class CodeGen {
public:
  CodeGen(type_graph::TypeGraph &typeGraph) : typeGraph_(typeGraph) { }
  void generate(drgn_type *drgnType);
  std::string ClassDecls(const std::vector<std::reference_wrapper<type_graph::Type>>& types);
  std::string ClassDefs(const std::vector<std::reference_wrapper<type_graph::Type>>& types);
  std::string GetSizeFuncs(const std::vector<std::reference_wrapper<type_graph::Type>>& types);

// TODO shouldn't need to be a template (but shouldn't be a set!)
template <typename T>
void loadConfig(const T &containerConfigPaths) {
  for (const auto &path : containerConfigPaths) {
    registerContainer(path);
  }
}

private:
  void registerContainer(const fs::path &path);
  std::string getContainerSizeFunc(const type_graph::Container &c);

  type_graph::TypeGraph &typeGraph_;
  std::vector<ContainerInfo> containerInfos_;
};
