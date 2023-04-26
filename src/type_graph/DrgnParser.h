#pragma once

#include <unordered_map>

// TODO sort out header ordering everywhere
#include "TypeGraph.h"
#include "Types.h"

struct drgn_type;
struct drgn_type_template_parameter;
struct ContainerInfo;

namespace type_graph {

class DrgnParser {
public:
  DrgnParser(TypeGraph &typeGraph, const std::vector<ContainerInfo> &containers, bool chaseRawPointers)
    : typeGraph_(typeGraph), containers_(containers), chaseRawPointers_(chaseRawPointers) { }
  Type *parse(struct drgn_type *root);

private:
  Type      *enumerateType(struct drgn_type *type);
  Container *enumerateContainer(struct drgn_type *type);
  Type      *enumerateClass(struct drgn_type *type);
  Enum      *enumerateEnum(struct drgn_type *type);
  Typedef   *enumerateTypedef(struct drgn_type *type);
  Type      *enumeratePointer(struct drgn_type *type);
  Array     *enumerateArray(struct drgn_type *type);
  Primitive *enumeratePrimitive(struct drgn_type *type);

  void enumerateTemplateParam(drgn_type_template_parameter *tparams,
                              size_t i,
                              std::vector<TemplateParam> &params);
  void enumerateClassTemplateParams(struct drgn_type *type,
                                    std::vector<TemplateParam> &params);
  void enumerateClassParents(struct drgn_type *type, std::vector<Parent> &parents);
  void enumerateClassMembers(struct drgn_type *type, std::vector<Member> &members);
  void enumerateClassFunctions(struct drgn_type *type, std::vector<Function> &functions);

  // Store a mapping of drgn types to type graph nodes for deduplication during
  // parsing. This stops us getting caught in cycles.
  std::unordered_map<struct drgn_type*, Type*> drgn_types_;

  // TODO define in .cpp
  template <typename T, typename ...Args>
  T *make_type(struct drgn_type *type, Args &&...args) {
    auto type_unique_ptr = std::make_unique<T>(std::forward<Args>(args)...);
    auto type_raw_ptr = type_unique_ptr.get();
    typeGraph_.add(std::move(type_unique_ptr));
    drgn_types_.insert({type, type_raw_ptr});
    return type_raw_ptr;
  }
  bool chasePointer() const;

  TypeGraph &typeGraph_;
  const std::vector<ContainerInfo> &containers_;
  int depth_;
  bool chaseRawPointers_;
};

class DrgnParserError : public std::runtime_error {
public:
  DrgnParserError(const std::string& msg)
    : std::runtime_error{msg} { }
  DrgnParserError(const std::string& msg, struct drgn_error *err)
    : std::runtime_error{msg + ": " + std::to_string(err->code) + " " + err->message}, err_(err) { }

  ~DrgnParserError();

private:
  struct drgn_error *err_ = nullptr;
};

} // namespace type_graph
