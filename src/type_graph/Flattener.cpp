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
  // WARNING: This does not work for virtual inheritance
  //
  // Given the following setup, with "Root" being the root type:
  //   struct C {
  //     int cMember;
  //   };
  //   struct BParent {
  //     int bParentMember;
  //   };
  //   struct B : BParent {
  //     int bMember;
  //     C c;
  //   };
  //   struct AParent {
  //     int aParentMember;
  //   };
  //   struct A : AParent {
  //     int aMember;
  //   };
  //   struct Root : B {
  //     A a;
  //     int rootMember;
  //   };
  //
  // We will transform the type graph into:
  //   struct C {
  //     int cMember;
  //   };
  //   struct BParent {
  //     int bParentMember;
  //   };
  //   struct B {
  //     int bParentMember;
  //     C c;
  //   };
  //   struct AParent {
  //     int aParentMember;
  //   }
  //   struct A {
  //     int aParentMember;
  //     int aMember;
  //   };
  //   struct Root {
  //     int bParentMember;
  //     int bMember;
  //     C c;
  //     A a;
  //     int rootMember;
  //   };

  // TODO alignment of parent classes

  // Flatten types referenced by member variables
  for (const auto &member : c.members) {
    visit(*member.type);
  }

  // Flatten parent classes into this class
  std::vector<Member> flattenedMembers;
  for (const auto &parent : c.parents) {
    visit(*parent.type);
    // TODO account for typedefs
    const Class &parentClass = dynamic_cast<Class&>(*parent.type);
    for (const auto &member : parentClass.members) {
      flattenedMembers.push_back(member);
      flattenedMembers.back().offset += parent.offset;
    }
  }
  c.parents.clear();

  flattenedMembers.insert(flattenedMembers.end(),
      c.members.begin(), c.members.end());
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
