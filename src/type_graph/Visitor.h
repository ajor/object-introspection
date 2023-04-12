#pragma once

#include "Types.h"

namespace type_graph {

/*
 * Visitor
 *
 * Abstract visitor base class.
 */
class Visitor {
public:
  virtual ~Visitor() = default;

#define X(OI_TYPE_NAME) virtual void visit(OI_TYPE_NAME &) = 0;
OI_TYPE_LIST
#undef X
};

/*
 * LazyVisitor
 *
 * Visitor base class which takes no action by default.
 */
class LazyVisitor : public Visitor {
public:
  virtual ~LazyVisitor() = default;

#define X(OI_TYPE_NAME) virtual void visit(OI_TYPE_NAME &) { }
OI_TYPE_LIST
#undef X
};

/*
 * RecursiveVisitor
 *
 * Visitor base class which recurses into types by default.
 */
class RecursiveVisitor : public Visitor {
public:
  virtual ~RecursiveVisitor() = default;
  virtual void visit(Type &) = 0;
  virtual void visit(Class &c) {
    for (const auto &param : c.templateParams) {
      visit(*param.type);
    }
    for (const auto &parent : c.parents) {
      visit(*parent.type);
    }
    for (const auto &mem : c.members) {
      visit(*mem.type);
    }
    for (const auto &child : c.children) {
      visit(child);
    }
  }
  virtual void visit(Container &c) {
    for (const auto &param : c.templateParams) {
      visit(*param.type);
    }
  }
  virtual void visit(Primitive &) { }
  virtual void visit(Enum &) { }
  virtual void visit(Array &a) {
    visit(*a.elementType());
  }
  virtual void visit(Typedef &td) {
    visit(*td.underlyingType());
  }
  virtual void visit(Pointer &p) {
    visit(*p.pointeeType());
  }
  virtual void visit(Dummy &) { }
  virtual void visit(DummyAllocator &d) {
    visit(d.allocType());
  }
};

/*
 * CodeGenVisitor
 *
 * Visitor base class for walking a type graph without modifying it.
 */
class CodeGenVisitor {
public:
  virtual ~CodeGenVisitor() = default;

  // TODO work out how to get rid of this "2"
  void visit2(const Type &type) {
    type.accept(*this);
  }

#define X(OI_TYPE_NAME) virtual void visit(const OI_TYPE_NAME &) { }
OI_TYPE_LIST
#undef X
};

} // namespace type_graph
