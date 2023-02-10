#pragma once

#include <ostream>
#include <unordered_map>

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
  void visit(Typedef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  bool prefix(Type *type=nullptr);
  void print_param(const TemplateParam &param);
  void print_parent(const Parent &parent);
  void print_member(const Member &member);

  std::ostream &out_;
  int depth_ = -1;
  int next_node_num = 0;
  std::unordered_map<Type*, int> node_nums;
};

} // namespace type_graph
