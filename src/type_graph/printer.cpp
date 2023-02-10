#include "printer.h"

namespace type_graph {

void Printer::print(Type &type) {
  depth_++;
  type.accept(*this);
  depth_--;
}

void Printer::visit(Class &c) {
  prefix(&c);
  std::string kind;
  switch (c.kind_) {
    case Class::Kind::Class:
      kind = "Class";
      break;
    case Class::Kind::Struct:
      kind = "Struct";
      break;
    case Class::Kind::Union:
      kind = "Union";
      break;
  }
  out_ << kind << ": " << c.name() << " (" << c.size() << ")" << std::endl;
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
  prefix(&c);
  out_ << "Container: " << c.name() << std::endl;
  for (const auto &param : c.template_params) {
    print_param(param);
  }
}

void Printer::visit(Enum &e) {
  prefix();
  out_ << "Enum: " << e.name() << " (" << e.size() << ")" << std::endl;
}

void Printer::visit(Primitive &p) {
  prefix();
  out_ << "Primitive: " << p.name() << std::endl;
}

void Printer::visit(Typedef &td) {
  prefix(&td);
  out_ << "Typedef: " << td.name() << std::endl;
  print(*td.underlying_type());
}

void Printer::visit(Pointer &p) {
  prefix(&p);
  out_ << "Pointer" << std::endl;

  if (auto it=node_nums.find(p.pointee_type()); it!=node_nums.end()) {
    // Cycle
    depth_++;
    prefix();
    int node_num = it->second;
    out_ << "[" << node_num << "]" << std::endl;
    depth_--;
    return;
  }
  print(*p.pointee_type());
}

void Printer::visit(Array &a) {
  prefix(&a);
  out_ << "Array: " << a.name() << std::endl;
  // TODO underlying type
}

void Printer::prefix(Type *type) {
  if (type) {
    int node_num = next_node_num++;
    out_ << "[" << node_num << "] "; // TODO pad numbers
    node_nums.insert({type, node_num});
  }
  else {
    // Extra padding
    out_ << "    "; // TODO make variable size
  }
  out_ << std::string(depth_*2, ' ');
}

void Printer::print_parent(const Parent &parent) {
  depth_++;
  prefix();
  out_ << "Parent (" << parent.offset << ")" << std::endl;
  print(*parent.type);
  depth_--;
}

void Printer::print_member(const Member &member) {
  depth_++;
  prefix();
  out_ << "Member: " << member.name << " (" << member.offset << ")" << std::endl;
  print(*member.type);
  depth_--;
}

void Printer::print_param(const TemplateParam &param) {
  depth_++;
  prefix();
  out_ << "Param" << std::endl;
  print(*param.type);
  depth_--;
}

} // namespace type_graph
