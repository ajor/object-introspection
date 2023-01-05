#pragma once

#include "types.h"
#include "visitor.h"

class StructDefinitions : public Visitor {
public:
  std::string def(Type &t) {
    t.accept(*this);
    return def_;
  }

  void visit(Class &c) override;
  void visit(Container &c) override;
  void visit(Enum &e) override;
  void visit(Primitive &p) override;
  void visit(TypeDef &td) override;
  void visit(Pointer &p) override;

private:
  std::string def_;
};
