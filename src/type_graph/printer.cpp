#include "printer.h"

namespace type_graph {

void Printer::print(Type &type) {
  // TODO first indent should be 0
  depth_++;
  type.accept(*this);
  depth_--;
}

void Printer::visit(Class &c) {
  out_ << indent() << "Class: " << c.name() << " (" << c.size() << ")" << std::endl;
  // TODO template parameters
  for (const auto &parent : c.parents) {
    print_parent(parent);
  }
  for (const auto &member : c.members) {
    print_member(member);
  }
  // TODO functions
}

void Printer::visit(Container &c) {
  out_ << indent() << "Container: " << c.name() << std::endl;
  for (const auto &param : c.template_params) {
    print_param(param);
  }
}

void Printer::visit(Enum &e) {
  out_ << indent() << "Enum: " << e.name() << " (" << e.size() << ")" << std::endl;
}

void Printer::visit(Primitive &p) {
  out_ << indent() << "Primitive: " << p.name() << std::endl;
}

void Printer::visit(TypeDef &td) {
  out_ << indent() << "TypeDef: " << td.name() << std::endl;
}

void Printer::visit(Pointer &p) {
  out_ << indent() << "Pointer" << std::endl;

  print(*p.pointee_type());
}

void Printer::visit(Array &a) {
  out_ << indent() << "Array: " << a.name() << std::endl;
}

std::string Printer::indent() const {
  return std::string(depth_*2, ' ');
}

void Printer::print_parent(const Parent &parent) {
  depth_++;
  out_ << indent() << "Parent (" << parent.offset << ")" << std::endl;
  print(*parent.type);
  depth_--;
}

void Printer::print_member(const Member &member) {
  depth_++;
  out_ << indent() << "Member: " << member.name << " (" << member.offset << ")" << std::endl;
  print(*member.type);
  depth_--;
}

void Printer::print_param(const TemplateParam &param) {
  depth_++;
  out_ << indent() << "Param" << std::endl;
  print(*param.type);
  depth_--;
}

} // namespace type_graph
