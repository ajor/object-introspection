#include "Flattener.h"

namespace type_graph {

void Flattener::flatten(const std::vector<Class*> &classes) {
  for (auto c : classes) {
    flatten_class(*c);
  }
}

void Flattener::flatten_class(Class &c) {
  flattened_members_ = {};
  offset_stack_ = {0};
  c.accept(*this);
  c.members = std::move(flattened_members_);
  c.parents.clear();
}

void Flattener::visit(Class &c) {
  // Members of a base class will be contiguous, but it's possible for derived
  // class members to be intersperced between embedded parent classes.
  //
  // e.g. Givin the original C++ classes:
  //   class Parent {
  //     int x;
  //     int y;
  //   };
  //   class Child : Parent {
  //     int a;
  //     int b;
  //   };
  //
  // The in memory (flattened) representation could be:
  //   class Child {
  //     int a;
  //     int x;
  //     int y;
  //     int b;
  //   };

  // Base offset of the class "c" in the class being flattened
  auto base_offset = offset_stack_.back();

  std::size_t member_idx = 0;
  std::size_t parent_idx = 0;
  // TODO is this loop necessary??
  while (member_idx < c.members.size() && parent_idx < c.parents.size()) {
    auto member_offset = c.members[member_idx].offset;
    auto parent_offset = c.parents[parent_idx].offset;
    if (member_offset < parent_offset) {
      // Add our own member
      auto member = c.members[member_idx++];
      member.offset += base_offset;
      flattened_members_.push_back(member);
    }
    else {
      // Add parent's members
      // If member_offset == parent_offset then the parent is empty. Also take this path.
      auto &parent = c.parents[parent_idx++];
      offset_stack_.push_back(base_offset + parent.offset);
      parent.type->accept(*this);
      offset_stack_.pop_back();
    }
  }
  while (member_idx < c.members.size()) {
    auto member = c.members[member_idx++];
    member.offset += base_offset;
    flattened_members_.push_back(member);
  }
  while (parent_idx < c.parents.size()) {
    auto &parent = c.parents[parent_idx++];
    offset_stack_.push_back(base_offset + parent.offset);
    parent.type->accept(*this);
    offset_stack_.pop_back();
  }

//  // TODO also walk members
//  for (const auto &mem : c.members) {
//    sort_type(mem.type);
//  }
//  for (const auto &parent : c.parents) {
//    sort_type(parent.type);
//  }
//  for (const auto &template_param : c.templateParams) {
//    sort_type(template_param.type);
//  }
}

void Flattener::visit(Container &c) {
}

void Flattener::visit(Enum &e) {
}

void Flattener::visit(Primitive &p) {
}

void Flattener::visit(Typedef &td) {
}

void Flattener::visit(Pointer &p) {
}

void Flattener::visit(Array &a) {
}

} // namespace type_graph
