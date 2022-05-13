#pragma once

#include <string>

#include "type_graph.h"
//#include "struct_definitions.h"

class OICodeGen2 {
public:
  OICodeGen2(const TypeGraph &type_graph) : type_graph_(type_graph) { }
  std::string ClassDecls();
  std::string ClassDefs();

private:
  const TypeGraph &type_graph_;
//  StructDefinitions defs_;
};
