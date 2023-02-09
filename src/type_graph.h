#pragma once

#include <memory>
#include <vector>

#include "types.h"

class TypeGraph {
public:
  void add(std::unique_ptr<Type> type) {
    types_.push_back(std::move(type));
    // TODO this could put types into the appropriate vectors if needed
  }

  // TODO
  const std::vector<Type*> types() const {
    std::vector<Type*> types;
    types.reserve(types_.size());
    for (auto &type : types_)
      types.push_back(type.get());
    return types;
  }

  // TODO maybe remove?
  const std::vector<Class*> &classes() const {
    return classes_;
  }

private:
  // Store all type objects in vectors for ownership. Order is not significant.
  std::vector<std::unique_ptr<Type>> types_;

  // TODO remove:
  // Store certain types in their own vectors for easy iteration.
  // There is some duplication of pointers with the types_ vector, but this way
  // we don't have to expose the owning unique_ptrs to whoever wants to iterate
  // over our types.
  std::vector<Class*> classes_;
  std::vector<TypeDef*> typedefs_;
};
