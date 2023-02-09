#pragma once

#include <string>

#include "type_graph/type_graph.h"

class OICodeGen2 {
public:
  OICodeGen2(const type_graph::TypeGraph &type_graph) : type_graph_(type_graph) { }
  std::string ClassDecls(const std::vector<type_graph::Type*>& types);
  std::string ClassDefs(const std::vector<type_graph::Type*>& types);
  std::string GetSizeFuncs(const std::vector<type_graph::Type*>& types);

private:
  const type_graph::TypeGraph &type_graph_;
//  StructDefinitions defs_;
};
