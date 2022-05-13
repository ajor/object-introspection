#pragma once

#include <unordered_set>
#include <vector>

#include "types.h"
#include "visitor.h"

/*
 * TypeFlattener
 *
 * Flattens classes by removing parents and adding their members directly into
 * derived classes.
 */
class TypeFlattener : public Visitor {
public:
  void flatten(const std::vector<Class*> &classes);

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(TypeDef &td) override;
  void visit(Pointer &p) override;
  void visit(Array &a) override;

private:
  std::vector<Member> flattened_members_;

  void flatten_class(Class &c);

  std::vector<uint64_t> offset_stack_;
};
