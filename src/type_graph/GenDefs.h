#pragma once

#include <string>

#include "Types.h"
#include "Visitor.h"

namespace type_graph {

class TypeGraph;

/*
 * GenDefs
 *
 * Generate C++ definitions for each visited type.
 */
class GenDefs : public LazyConstVisitor {
public:
  static void run(TypeGraph &typeGraph, std::string &out);

  GenDefs(std::string &out) : out_(out) { }

  void visit(const Class &c) override;
  void visit(const Typedef &td) override;

private:
  std::string &out_;
};

} // namespace type_graph
