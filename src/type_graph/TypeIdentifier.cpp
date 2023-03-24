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
  for (const auto &containerInfo : containers_) {
    if (!std::regex_search(c.name(), containerInfo.matcher)) {
      continue;
    }

    auto *c = make_type<Container>(containerInfo);

    const auto &stubParams = containerInfo.stubTemplateParams;
    for (size_t i = 0; i < c->templateParams.size(); i++) {
      if (std::find(stubParams.begin(), stubParams.end(), i) != stubParams.end()) {
        const auto &param = c->templateParams[i];
        auto *dummy = make_type<Dummy>(param.type->size(), param.type->align());
        c->templateParams[i] = dummy;
      }
    }

    // TODO replace current node with "c"
  }

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

void TypeIdentifier::visit(Container &c) {
  // TODO will containers exist at this point?
  // maybe don't need this function
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
