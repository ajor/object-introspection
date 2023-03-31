#include "GenDefs.h"

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

void GenDefs::run(TypeGraph &typeGraph, std::string &out) {
  GenDefs gendefs{out};
  for (const auto &type : typeGraph.finalTypes) {
    gendefs.visit2(type);
  }
}

void GenDefs::visit(const Class &c) {
  out_ += "struct " + c.name() + " {\n";
  for (const auto &mem : c.members) {
    out_ += "  " + mem.type->name() + " " + mem.name + ";\n";
  }
  out_ += "};\n";
}

void GenDefs::visit(const Typedef &td) {
  out_ += "using " + td.name() + " = " + td.underlyingType()->name() + ";\n";
}

} // namespace type_graph
