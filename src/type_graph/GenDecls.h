#pragma once

#include <string>

#include "Types.h"
#include "Visitor.h"

namespace type_graph {

class TypeGraph;

/*
 * GenDecls
 *
 * Generate C++ declarations for each visited type.
 */
class GenDecls : public LazyConstVisitor {
public:
  static void run(TypeGraph &typeGraph, std::string &out);

  GenDecls(std::string &out) : out_(out) { }

  void visit(const Class &c) override;
  void visit(const Enum &e) override;
  void visit(const Typedef &td) override;

private:
  std::string &out_;
};

} // namespace type_graph
