#include "GenDecls.h"

#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

void GenDecls::run(TypeGraph &typeGraph, std::string &out) {
  GenDecls gendecls{out};
  for (const auto &type : typeGraph.finalTypes) {
    gendecls.visit2(type);
  }
}

void GenDecls::visit(const Class &c) {
  if (c.kind() == Class::Kind::Union)
    out_ += "union ";
  else
    out_ += "struct ";
  out_ += c.name() + ";\n";
}

void GenDecls::visit(const Enum &e) {
  out_ += "using " + e.name() + " = ";
  switch (e.size()) {
    case 8:
      out_ += "uint64_t";
      break;
    case 4:
      out_ += "uint32_t";
      break;
    case 2:
      out_ += "uint16_t";
      break;
    case 1:
      out_ += "uint8_t";
      break;
    default:
      abort(); // TODO
  }
  out_ += ";\n";
}

void GenDecls::visit(const Typedef &td) {
  out_ += "using " + td.name() + " = " + td.underlyingType()->name() + ";\n";
}

} // namespace type_graph
