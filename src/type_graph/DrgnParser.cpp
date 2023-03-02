#include "DrgnParser.h"
#include "ContainerInfo.h"

extern "C" {
#include <drgn.h>
}

#include <regex>

namespace type_graph {
namespace {

uint64_t get_drgn_type_size(struct drgn_type *type) {
  uint64_t size;
  struct drgn_error *err = drgn_type_sizeof(type, &size);
  if (err)
    abort(); // TODO
  return size;
}

Primitive::Kind primitiveIntKind(struct drgn_type *type) {
  auto size = get_drgn_type_size(type);

  bool is_signed = type->_private.is_signed; // TODO shouldn't access private member
  switch (size) {
    case 1:
      return is_signed ? Primitive::Kind::Int8 : Primitive::Kind::UInt8;
    case 2:
      return is_signed ? Primitive::Kind::Int16 : Primitive::Kind::UInt16;
    case 4:
      return is_signed ? Primitive::Kind::Int32 : Primitive::Kind::UInt32;
    case 8:
      return is_signed ? Primitive::Kind::Int64 : Primitive::Kind::UInt64;
    default:
      abort(); // TODO
  }
}

Primitive::Kind primitiveFloatKind(struct drgn_type *type) {
  auto size = get_drgn_type_size(type);

  switch (size) {
    case 4:
      return Primitive::Kind::Float32;
    case 8:
      return Primitive::Kind::Float64;
    case 16:
      return Primitive::Kind::Float128;
    default:
      abort(); // TODO
  }
}

} // namespace

// TODO type stubs

Type *DrgnParser::parse(struct drgn_type *root) {
  depth_ = 0;
  return enumerateType(root);
}

Type *DrgnParser::enumerateType(struct drgn_type *type) {
  // Avoid re-enumerating an already-processsed type
  if (auto it = drgn_types_.find(type); it != drgn_types_.end())
    return it->second;

  enum drgn_type_kind kind = drgn_type_kind(type);
  Type *t = nullptr;
  depth_++;
  switch (kind) {
    case DRGN_TYPE_CLASS:
    case DRGN_TYPE_STRUCT:
    case DRGN_TYPE_UNION:
      t = enumerateClass(type);
      break;
    case DRGN_TYPE_ENUM:
      t = enumerateEnum(type);
      break;
    case DRGN_TYPE_TYPEDEF:
      t = enumerateTypedef(type);
      break;
    case DRGN_TYPE_POINTER:
      t = enumeratePointer(type);
      break;
    case DRGN_TYPE_ARRAY:
      t = enumerateArray(type);
      break;
    case DRGN_TYPE_INT:
    case DRGN_TYPE_BOOL:
    case DRGN_TYPE_FLOAT:
      t = enumeratePrimitive(type);
      break;
    case DRGN_TYPE_VOID:
    case DRGN_TYPE_FUNCTION:
      // Do nothing
      break;
      // TODO ensure that missing cases are a compile error
  }
  depth_--;

  return t;
}

Container *DrgnParser::enumerateContainer(struct drgn_type *type) {
  char *nameStr = nullptr;
  size_t length = 0;
  auto *err = drgn_type_fully_qualified_name(type, &nameStr, &length);
  if (err != nullptr || nameStr == nullptr) {
    return nullptr;
  }

  std::string name{nameStr}; // TODO needs derefence??
  for (const auto &containerInfo : containers_) {
    if (!std::regex_search(nameStr, containerInfo.matcher)) {
      continue;
    }

    auto *c = make_type<Container>(type, containerInfo);
    enumerateContainerTemplateParams(type, c->templateParams, containerInfo.stubTemplateParams);
    return c;
  }
  return nullptr;
}

Type *DrgnParser::enumerateClass(struct drgn_type *type) {
  auto *container = enumerateContainer(type);
  if (container)
    return container;

  std::string type_name;
  const char *type_tag = drgn_type_tag(type);
  if (type_tag)
    type_name = std::string(type_tag);
  // else this is an anonymous type

  auto size = get_drgn_type_size(type);

  Class::Kind kind;
  switch (drgn_type_kind(type)) {
    case DRGN_TYPE_CLASS:
      kind = Class::Kind::Class;
      break;
    case DRGN_TYPE_STRUCT:
      kind = Class::Kind::Struct;
      break;
    case DRGN_TYPE_UNION:
      kind = Class::Kind::Union;
      break;
    default:
      abort(); // TODO
  }

  auto c = make_type<Class>(type, kind, type_name, size);

  //classes_.push_back(c);

  enumerateClassTemplateParams(type, c->templateParams);
  enumerateClassParents(type, c->parents);
  enumerateClassMembers(type, c->members);
  enumerateClassFunctions(type, c->functions);

  return c;
}

void DrgnParser::enumerateClassParents(struct drgn_type *type, std::vector<Parent> &parents) {
  assert(parents.empty());
  size_t num_parents = drgn_type_num_parents(type);
  parents.reserve(num_parents);

  struct drgn_type_template_parameter *drgn_parents = drgn_type_parents(type);

  for (size_t i = 0; i < num_parents; i++) {
    struct drgn_qualified_type parent_qual_type;
    struct drgn_error *err = drgn_template_parameter_type(&drgn_parents[i], &parent_qual_type);
    if (err) {
      abort(); // TODO throw an exception instead
    }

    auto ptype = enumerateType(parent_qual_type.type);
    uint64_t poffset = drgn_parents[i].bit_offset / 8;
    Parent p(ptype, poffset);
    parents.push_back(p);
  }

  std::sort(parents.begin(), parents.end(),
            [](const auto &a, const auto &b) {
              return a.offset < b.offset;
            });
}

void DrgnParser::enumerateClassMembers(struct drgn_type *type, std::vector<Member> &members) {
  assert(members.empty());
  size_t num_members = drgn_type_num_members(type);
  members.reserve(num_members);

  struct drgn_type_member *drgn_members = drgn_type_members(type);
  for (size_t i = 0; i < num_members; i++) {
    struct drgn_qualified_type member_qual_type;
    uint64_t bit_field_size;
    struct drgn_error *err =
        drgn_member_type(&drgn_members[i], &member_qual_type, &bit_field_size);
    if (err)
      abort(); // TODO

    struct drgn_type *member_type = member_qual_type.type;

//    if (err || !isDrgnSizeComplete(member_qual_type.type)) {
//      if (err) {
//        LOG(ERROR) << "Error when looking up member type " << err->code << " "
//                   << err->message << " " << typeName << " " << drgn_members[i].name;
//      }
//      VLOG(1) << "Type " << typeName
//              << " has an incomplete member; stubbing...";
//      knownDummyTypeList.insert(type);
//      isStubbed = true;
//      return;
//    }
    std::string member_name = "";
    if (drgn_members[i].name)
      member_name = drgn_members[i].name;

    // TODO bitfields

    auto mtype = enumerateType(member_type);
    uint64_t moffset = drgn_members[i].bit_offset / 8;

    Member m(mtype, member_name, moffset); // TODO
    members.push_back(m);
  }

  std::sort(members.begin(), members.end(),
            [](const auto &a, const auto &b) {
              return a.offset < b.offset;
            });
}

void DrgnParser::enumerateTemplateParam(drgn_type_template_parameter *tparams,
                                        size_t i,
                                        std::vector<TemplateParam> &params) {
  struct drgn_qualified_type tparamQualType;
  struct drgn_error *err = drgn_template_parameter_type(&tparams[i], &tparamQualType);
  if (err)
    abort(); // TODO

  struct drgn_type *tparamType = tparamQualType.type;

  auto ttype = enumerateType(tparamType);
  params.emplace_back(ttype);
}

void DrgnParser::stubTemplateParam(drgn_type_template_parameter *tparams,
                                   size_t i,
                                   std::vector<TemplateParam> &params) {
  struct drgn_qualified_type tparamQualType;
  struct drgn_error *err = drgn_template_parameter_type(&tparams[i], &tparamQualType);
  if (err)
    abort(); // TODO

  struct drgn_type *tparamType = tparamQualType.type;
  auto size = drgn_type_size(tparamType);
  auto align = 0; // TODO

  auto *dummy = make_type<Dummy>(nullptr, size, align);
  params.emplace_back(dummy);
}

void DrgnParser::enumerateContainerTemplateParams(struct drgn_type *type,
    std::vector<TemplateParam> &params,
    const std::vector<size_t> &stubParams) {
  assert(params.empty());
  size_t numParams = drgn_type_num_template_parameters(type);
  params.reserve(numParams - stubParams.size());

  struct drgn_type_template_parameter *tparams = drgn_type_template_parameters(type);
  for (size_t i = 0; i < numParams; i++) {
    if (std::find(stubParams.begin(), stubParams.end(), i) != stubParams.end()) {
      // Stub this template parameter
      stubTemplateParam(tparams, i, params);
    }
    else {
      enumerateTemplateParam(tparams, i, params);
    }
  }
}

void DrgnParser::enumerateClassTemplateParams(struct drgn_type *type,
    std::vector<TemplateParam> &params) {
  assert(params.empty());
  size_t numParams = drgn_type_num_template_parameters(type);
  params.reserve(numParams);

  struct drgn_type_template_parameter *tparams = drgn_type_template_parameters(type);
  for (size_t i = 0; i < numParams; i++) {
    enumerateTemplateParam(tparams, i, params);
  }
}

void DrgnParser::enumerateClassFunctions(struct drgn_type *type, std::vector<Function> &functions) {
  assert(functions.empty());
  size_t num_functions = drgn_type_num_functions(type);
  functions.reserve(num_functions);

  drgn_type_member_function *drgn_functions = drgn_type_functions(type);
  for (size_t i = 0; i < num_functions; i++) {
    drgn_qualified_type t{};
    if (auto *err = drgn_member_function_type(&drgn_functions[i], &t)) {
//      LOG(ERROR) << "Error when looking up member function for type " << type
//                 << " err " << err->code << " " << err->message;
      drgn_error_destroy(err);
      continue;
    }

    auto virtuality = drgn_type_virtuality(t.type);
    std::string name = drgn_type_name(t.type);
    Function f(name, virtuality);
    functions.push_back(f);
  }
}

Enum *DrgnParser::enumerateEnum(struct drgn_type *type) {
  // TODO anonymous enums
  std::string name = drgn_type_tag(type);

  uint64_t size;
  struct drgn_error *err = drgn_type_sizeof(type, &size);
  if (err)
    abort(); // TODO

  return make_type<Enum>(type, name, size);
}

Typedef *DrgnParser::enumerateTypedef(struct drgn_type *type) {
  std::string name = drgn_type_name(type);
  // TODO anonymous typedefs?

  struct drgn_type *underlyingType = drgn_type_type(type).type;
  auto t = enumerateType(underlyingType);
  return make_type<Typedef>(type, name, t);
}

bool isDrgnSizeComplete(struct drgn_type *type) {
  uint64_t sz;
  struct drgn_error *err = drgn_type_sizeof(type, &sz);
  return err == nullptr;

  // TODO this ignores sizeMap
}

// TODO will have to do something about pointers in the visitor classes
// - sometimes they must be followed, sometimes not?

Type *DrgnParser::enumeratePointer(struct drgn_type *type) {
  if (!chasePointer()) {
    // TODO dodgy nullptr - primitives should be handled as singletons
    return make_type<Primitive>(nullptr, Primitive::Kind::UIntPtr);
  }

  struct drgn_type *pointeeType = drgn_type_type(type).type;

  // TODO why was old CodeGen following funciton pointers?

  if (!isDrgnSizeComplete(pointeeType)) {
    return make_type<Primitive>(nullptr, Primitive::Kind::UIntPtr);
  }

  Type *t = enumerateType(pointeeType);
  return make_type<Pointer>(type, t);
}

Array *DrgnParser::enumerateArray(struct drgn_type *type) {
  struct drgn_type *elementType = drgn_type_type(type).type;
  uint64_t len = drgn_type_length(type);
  auto t = enumerateType(elementType);
  return make_type<Array>(type, t, len);
}

// TODO deduplication of primitive types (also remember they're not only created here)
Primitive *DrgnParser::enumeratePrimitive(struct drgn_type *type) {
  Primitive::Kind kind;
  switch (drgn_type_kind(type)) {
    case DRGN_TYPE_INT:
      kind = primitiveIntKind(type);
      break;
    case DRGN_TYPE_FLOAT:
      kind = primitiveFloatKind(type);
      break;
    case DRGN_TYPE_BOOL:
      kind = Primitive::Kind::Bool;
      break;
    default:
      abort(); // TODO
  }
  return make_type<Primitive>(type, kind);
}

bool DrgnParser::chasePointer() const {
  // Chase top-level pointers
  return depth_ == 1;
  // TODO obey chase-raw-pointers command line argument
}

} // namespace type_graph
