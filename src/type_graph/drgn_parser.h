#pragma once

#include <unordered_map>

#include "type_graph.h"
#include "types.h"

struct drgn_type;

namespace type_graph {

// TODO rename DrgnParser
class DrgnParser {
public:
  DrgnParser(TypeGraph &type_graph)
    : type_graph_(type_graph) { }
  Type *parse(struct drgn_type *root);

private:
  Type *enumerateType(struct drgn_type *type);
  Container *enumerateContainer(struct drgn_type *type);

  Type  *enumerateClass(struct drgn_type *type);
  void   enumerateClassParents(struct drgn_type *type, std::vector<Parent> &parents);
  void   enumerateClassMembers(struct drgn_type *type, std::vector<Member> &members);
  void   enumerateClassTemplateParams(struct drgn_type *type,
                                      std::vector<TemplateParam> &params);
  void   enumerateClassFunctions(struct drgn_type *type, std::vector<Function> &functions);

  Enum      *enumerateEnum(struct drgn_type *type);
  Typedef   *enumerateTypedef(struct drgn_type *type);
  Pointer   *enumeratePointer(struct drgn_type *type);
  Array     *enumerateArray(struct drgn_type *type);
  Primitive *enumeratePrimitive(struct drgn_type *type);

  // Store a mapping of drgn types to type graph nodes for deduplication during
  // parsing. This stops us getting caught in cycles.
  std::unordered_map<struct drgn_type*, Type*> drgn_types_;

  // TODO define in .cpp
  template <typename T, typename ...Args>
  T *make_type(struct drgn_type *type, Args &&...args) {
    auto type_unique_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto type_raw_ptr = type_unique_ptr.get();
    type_graph_.add(std::move(type_unique_ptr));
    drgn_types_.insert({type, type_raw_ptr});
    return type_raw_ptr;
  }

  // TODO deuplicate names after type tree has been built

  // TODO padding type?

  TypeGraph &type_graph_;
};

} // namespace type_graph
