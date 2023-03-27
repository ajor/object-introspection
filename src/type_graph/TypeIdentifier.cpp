#include "TypeIdentifier.h"

#include "ContainerInfo.h"

namespace type_graph {

Pass TypeIdentifier::createPass(const std::vector<ContainerInfo> &containers) {
  // TODO is it ok to capture containers like this?
  auto fn = [&containers](TypeGraph &typeGraph) {
    TypeIdentifier typeId{typeGraph, containers};
    for (auto &type : typeGraph.rootTypes()) {
      typeId.visit(type);
    }
  };

  return Pass("TypeIdentifier", fn);
}

void TypeIdentifier::visit(Type &type) {
  if (visited_.count(&type) != 0)
    return;

  visited_.insert(&type);
  type.accept(*this);
}

void TypeIdentifier::visit(Class &c) {
  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
  for (const auto &parent : c.parents) {
    visit(*parent.type); // TODO should this pass happen after flattening?
  }
  for (const auto &member : c.members) {
    visit(*member.type);
  }
}

namespace {
bool isAllocator(Type *t) {
  auto *c = dynamic_cast<Class*>(t);
  if (!c)
    return false;

  // Maybe add more checks for an allocator.
  // For now, just test for the presence of an "allocate" function
  for (const auto &func : c->functions) {
    if (func.name == "allocate") {
      return true;
    }
  }
  return false;
}
} // namespace

void TypeIdentifier::visit(Container &c) {
  // TODO will containers exist at this point?
  // maybe don't need this function

//  auto *c = make_type<Container>(containerInfo);

  const auto &stubParams = c.containerInfo_.stubTemplateParams;
  // TODO these two arrays could be looped over in sync for better performance
  for (size_t i = 0; i < c.templateParams.size(); i++) {
    if (std::find(stubParams.begin(), stubParams.end(), i) != stubParams.end()) {
      const auto &param = c.templateParams[i];
      if (isAllocator(param.type)) {
        auto *allocator = dynamic_cast<Class*>(param.type); // TODO please don't do this...
        const auto &typeToAllocate = *allocator->templateParams.at(0).type;
        auto *dummy = make_type<DummyAllocator>(typeToAllocate, param.type->size(), param.type->align());
        c.templateParams[i] = dummy;
      }
      else {
        auto *dummy = make_type<Dummy>(param.type->size(), param.type->align());
        c.templateParams[i] = dummy;
      }
    }
  }

  // TODO replace current node with "c" (if we want to transform classes into containers here)

  for (const auto &param : c.templateParams) {
    visit(*param.type);
  }
}

void TypeIdentifier::visit(Array &a) {
  visit(*a.elementType());
}

void TypeIdentifier::visit(Typedef &td) {
  visit(*td.underlyingType());
}

void TypeIdentifier::visit(Pointer &p) {
  visit(*p.pointeeType());
}

} // namespace type_graph
