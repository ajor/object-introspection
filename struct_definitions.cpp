#include "struct_definitions.h"

void StructDefinitions::visit(Class &c) {
  def_ += c.name() + ";\n";
}

void StructDefinitions::visit(Container &c) {
}

void StructDefinitions::visit(Enum &e) {
}

void StructDefinitions::visit(Primitive &p) {
}

void StructDefinitions::visit(TypeDef &td) {
}

void StructDefinitions::visit(Pointer &p) {
}
