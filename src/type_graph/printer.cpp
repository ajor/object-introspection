#include "printer.h"

namespace type_graph {

void Printer::print(Type &type) {
  type.accept(*this);
}

void Printer::visit(Class &c) {
  out_ << "Class: " << c.name() << std::endl;
//  .type->accept(*this);
}

void Printer::visit(Container &c) {
  out_ << "Container: " << c.name() << std::endl;
}

void Printer::visit(Enum &e) {
  out_ << "Enum: " << e.name() << std::endl;
}

void Printer::visit(Primitive &p) {
  out_ << "Primitive: " << p.name() << std::endl;
}

void Printer::visit(TypeDef &td) {
  out_ << "TypeDef: " << td.name() << std::endl;
}

void Printer::visit(Pointer &p) {
  out_ << "Pointer: " << p.name() << std::endl;
}

void Printer::visit(Array &a) {
  out_ << "Array: " << a.name() << std::endl;
}

} // namespace type_graph
