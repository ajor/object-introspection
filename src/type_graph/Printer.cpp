#include "Printer.h"

namespace type_graph {

void Printer::print(Type &type) {
  depth_++;
  type.accept(*this);
  depth_--;
}

void Printer::visit(Class &c) {
  if (prefix(&c))
    return;

  std::string kind;
  switch (c.kind()) {
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
  out_ << kind << ": " << c.name() << " (size: " << c.size() << align_str(c.align());
  if (c.packed()) {
    out_ << ", packed";
  }
  out_ << ")" << std::endl;
  for (const auto &param : c.templateParams) {
    print_param(param);
  }
  for (const auto &parent : c.parents) {
    print_parent(parent);
  }
  for (const auto &member : c.members) {
    print_member(member);
  }
  for (const auto &function : c.functions) {
    print_function(function);
  }
  for (auto &child : c.children) {
    print_child(child);
  }
}

void Printer::visit(Container &c) {
  if (prefix(&c))
    return;

  out_ << "Container: " << c.name() << " (size: " << c.size() << ")" << std::endl;
  for (const auto &param : c.templateParams) {
    print_param(param);
  }
}

void Printer::visit(Primitive &p) {
  prefix();
  out_ << "Primitive: " << p.name() << std::endl;
}

void Printer::visit(Enum &e) {
  prefix();
  out_ << "Enum: " << e.name() << " (size: " << e.size() << ")" << std::endl;
}

void Printer::visit(Array &a) {
  if (prefix(&a))
    return;

  out_ << "Array: (length: " << a.len() << ")" << std::endl;
  print(*a.elementType());
}

void Printer::visit(Typedef &td) {
  if (prefix(&td))
    return;

  out_ << "Typedef: " << td.name() << std::endl;
  print(*td.underlyingType());
}

void Printer::visit(Pointer &p) {
  if (prefix(&p))
    return;

  out_ << "Pointer" << std::endl;
  print(*p.pointeeType());
}

void Printer::visit(Dummy &d) {
  prefix();
  out_ << "Dummy (size: " << d.size() << align_str(d.align()) << ")" << std::endl;
}

void Printer::visit(DummyAllocator &d) {
  prefix();
  out_ << "DummyAllocatorTODO (size: " << d.size() << align_str(d.align()) << ")" << std::endl;
}

bool Printer::prefix(Type *type) {
  if (type) {
    if (auto it=node_nums.find(type); it!=node_nums.end()) {
      // Node has already been printed - print a reference to it this time
      out_ << std::string(depth_*2, ' ');
      int node_num = it->second;
      out_ << "    [" << node_num << "]" << std::endl;
      return true;
    }

    int node_num = next_node_num++;
    out_ << "[" << node_num << "] "; // TODO pad numbers
    node_nums.insert({type, node_num});
  }
  else {
    // Extra padding
    out_ << "    "; // TODO make variable size
  }
  out_ << std::string(depth_*2, ' ');
  return false;
}

void Printer::print_param(const TemplateParam &param) {
  depth_++;
  prefix();
  out_ << "Param" << std::endl;
  if (param.value) {
    print_value(*param.value);
  }
  else {
    print(*param.type);
  }
  depth_--;
}

void Printer::print_parent(const Parent &parent) {
  depth_++;
  prefix();
  out_ << "Parent (offset: " << parent.offset << ")" << std::endl;
  print(*parent.type);
  depth_--;
}

void Printer::print_member(const Member &member) {
  depth_++;
  prefix();
  out_ << "Member: " << member.name << " (offset: " << member.offset << align_str(member.align) << ")" << std::endl;
  print(*member.type);
  depth_--;
}

void Printer::print_function(const Function &function) {
  depth_++;
  prefix();
  out_ << "Function: " << function.name;
  if (function.virtuality != 0)
    out_ << " (virtual)";
  out_ << std::endl;
  depth_--;
}

void Printer::print_child(Type &child) {
  depth_++;
  prefix();
  out_ << "Child:" << std::endl;
  print(child);
  depth_--;
}

void Printer::print_value(const std::string &value) {
  depth_++;
  prefix();
  out_ << "Value: " << value << std::endl;
  depth_--;
}

std::string Printer::align_str(uint64_t align) {
  if (align == 0)
    return "";
  return ", align: " + std::to_string(align);
}

} // namespace type_graph
