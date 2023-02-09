#pragma once

#include <string>

#include "type_graph.h"
//#include "struct_definitions.h"

class OICodeGen2 {
public:
  OICodeGen2(const TypeGraph &type_graph) : type_graph_(type_graph) { }
  std::string ClassDecls(const std::vector<Type*>& types);
  std::string ClassDefs(const std::vector<Type*>& types);
  std::string GetSizeFuncs(const std::vector<Type*>& types);

private:
  const TypeGraph &type_graph_;
//  StructDefinitions defs_;
};
