#include "RemoveTopLevelPointer.h"

#include "TypeGraph.h"

namespace type_graph {

Pass RemoveTopLevelPointer::createPass() {
  auto fn = [](TypeGraph &typeGraph) {
    RemoveTopLevelPointer pass;
    pass.removeTopLevelPointers(typeGraph.rootTypes());
  };

  return Pass("RemoveTopLevelPointer", fn);
}

void RemoveTopLevelPointer::removeTopLevelPointers(std::vector<std::reference_wrapper<Type>> &types) {
  for (size_t i=0; i<types.size(); i++) {
    Type &type = types[i];
    topLevelType_ = &type;
    type.accept(*this);
    types[i] = *topLevelType_;
  }
}

void RemoveTopLevelPointer::visit(Pointer &p) {
  topLevelType_ = p.pointeeType();
}

} // namespace type_graph
