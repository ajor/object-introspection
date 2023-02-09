#pragma once

#include <ostream>

#include "types.h"
#include "visitor.h"

namespace type_graph {

/*
 * Printer
 */
class Printer : public Visitor {
public:
  Printer(std::ostream &out) : out_(out) { }
  void print(Type &type);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(TypeDef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  std::string indent() const;
  void print_parent(const Parent &parent);
  void print_member(const Member &member);
  void print_param(const TemplateParam &param);
  std::ostream &out_;
  int depth_ = -1;
};

} // namespace type_graph
