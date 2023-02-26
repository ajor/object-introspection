#include "Flattener.h"

// TODO remove?
#include "TypeGraph.h"

namespace type_graph {

//Pass Flattener::createPass() {
//  return [](Type &root, TypeGraph &typeGraph) {
//    Flattener flattener;
//    flattener.flatten(typeGraph);
//
////    for (auto &root : typeGraph.rootTypes()) {
////      root.accept(flattener);
////    }
////    flattener.flatten({&root}); // TODO this should take a list of nodes, not just root
//  };
//}

void Flattener::flatten(std::vector<std::reference_wrapper<Type>> types) {
  for (auto &type : types) {
    visit(type);
  }
}

void Flattener::visit(Type &type) {
  if (visited_.count(&type) != 0)
    return;

  visited_.insert(&type);
  type.accept(*this);
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
  //    TODO comment about virtual inheritance

  // TODO alignment of parent classes

  // Flatten types referenced by member variables
  for (const auto &member : c.members) {
    visit(*member.type);
  }

  // Flatten parent types
  for (const auto &parent : c.parents) {
    visit(*parent.type);
  }

  // Pull member variables from flattened parents into this class
  std::vector<Member> flattenedMembers;

  std::size_t member_idx = 0;
  std::size_t parent_idx = 0;
  while (member_idx < c.members.size() && parent_idx < c.parents.size()) {
    auto member_offset = c.members[member_idx].offset;
    auto parent_offset = c.parents[parent_idx].offset;
    if (member_offset < parent_offset) {
      // Add our own member
      const auto &member = c.members[member_idx++];
      flattenedMembers.push_back(member);
    }
    else {
      // Add parent's members
      // If member_offset == parent_offset then the parent is empty. Also take this path.
      const auto &parent = c.parents[parent_idx++];
      // TODO account for typedefs
      const Class &parentClass = dynamic_cast<Class&>(*parent.type);
      for (const auto &member : parentClass.members) {
        flattenedMembers.push_back(member);
        flattenedMembers.back().offset += parent.offset;
      }
    }
  }
  while (member_idx < c.members.size()) {
    const auto &member = c.members[member_idx++];
    flattenedMembers.push_back(member);
  }
  while (parent_idx < c.parents.size()) {
    const auto &parent = c.parents[parent_idx++];
    // TODO account for typedefs
    const Class &parentClass = dynamic_cast<Class&>(*parent.type);
    for (const auto &member : parentClass.members) {
      flattenedMembers.push_back(member);
      flattenedMembers.back().offset += parent.offset;
    }
  }

  c.parents.clear();
  c.members = std::move(flattenedMembers);
}

void Flattener::visit(Container &c) {
  // Containers themselves don't need to be flattened, but their template
  // parameters might need to be
  for (const auto &templateParam : c.templateParams) {
    visit(*templateParam.type);
  }
}

void Flattener::visit(Enum &e) {
}

void Flattener::visit(Primitive &p) {
}

void Flattener::visit(Typedef &td) {
  // TODO flatten typedefs
  visit(*td.underlyingType());
}

void Flattener::visit(Pointer &p) {
  visit(*p.pointeeType());
}

void Flattener::visit(Array &a) {
  visit(*a.elementType());
}

} // namespace type_graph
