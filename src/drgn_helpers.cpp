#include "drgn_helpers.h"

#include <glog/logging.h>

namespace drgn_helpers {

void getDrgnArrayElementType(drgn_type *type,
                             drgn_type **outElemType,
                             size_t &outNumElems) {
  size_t elems = 1;

  // for multi dimensional arrays
  auto *arrayElementType = type;
  while (drgn_type_kind(arrayElementType) == DRGN_TYPE_ARRAY) {
    size_t l = drgn_type_length(arrayElementType);
    elems *= l;
    auto qtype = drgn_type_type(arrayElementType);
    arrayElementType = qtype.type;
  }

  *outElemType = arrayElementType;
  outNumElems = elems;
}

std::string typeToName(drgn_type *type) {
  std::string typeName;
  if (drgn_type_has_tag(type)) {
    const char *typeTag = drgn_type_tag(type);
    if (typeTag != nullptr) {
      typeName = typeTag;
    } else {
      typeName = type->_private.oi_name;
    }
    // TODO: Lookup unnamed union in type->string flag
  } else if (drgn_type_has_name(type)) {
    typeName = drgn_type_name(type);
  } else if (drgn_type_kind(type) == DRGN_TYPE_POINTER) {
    char *defStr = nullptr;
    drgn_qualified_type qtype = {type, {}};
    if (drgn_format_type_name(qtype, &defStr) != nullptr) {
      LOG(ERROR) << "Failed to get formatted string for " << type;
      typeName = "";
    } else {
      typeName.assign(defStr);
      free(defStr);
    }
  } else if (drgn_type_kind(type) == DRGN_TYPE_VOID) {
    return "void";
  } else if (drgn_type_kind(type) == DRGN_TYPE_ARRAY) {
    size_t elems = 1;
    drgn_type *arrayElementType = nullptr;
    getDrgnArrayElementType(type, &arrayElementType, elems);

    if (drgn_type_has_name(arrayElementType)) {
      typeName = drgn_type_name(arrayElementType);
    } else if (drgn_type_has_tag(arrayElementType)) {
      typeName = drgn_type_tag(arrayElementType);
    } else {
      LOG(ERROR) << "Failed4 to get typename ";
      return "";
    }
  } else {
    LOG(ERROR) << "Failed3 to get typename ";
    return "";
  }
  return typeName;
}

} // namespace drgn_helpers
