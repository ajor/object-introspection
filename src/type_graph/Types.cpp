#include "Types.h"

#include "Visitor.h"

namespace type_graph {

#define X(OI_TYPE_NAME) \
  void OI_TYPE_NAME::accept(Visitor &v) { v.visit(*this); } \
  void OI_TYPE_NAME::accept(CodeGenVisitor &v) const { v.visit(*this); }
OI_TYPE_LIST
#undef X

  // TODO enable error if not all switch cases are handled
std::string Primitive::name() const {
  switch (kind_) {
    case Kind::Int8:
      return "int8_t";
    case Kind::Int16:
      return "int16_t";
    case Kind::Int32:
      return "int32_t";
    case Kind::Int64:
      return "int64_t";
    case Kind::UInt8:
      return "uint8_t";
    case Kind::UInt16:
      return "uint16_t";
    case Kind::UInt32:
      return "uint32_t";
    case Kind::UInt64:
      return "uint64_t";
    case Kind::Float32:
      return "float";
    case Kind::Float64:
      return "double";
    case Kind::Float80:
      abort();
    case Kind::Float128:
      return "long double";
    case Kind::Bool:
      return "bool";
    case Kind::UIntPtr:
      return "uintptr_t";
    case Kind::Void:
      return "void";
  }
}

std::size_t Primitive::size() const {
  switch (kind_) {
    case Kind::Int8:
      return 1;
    case Kind::Int16:
      return 2;
    case Kind::Int32:
      return 4;
    case Kind::Int64:
      return 8;
    case Kind::UInt8:
      return 1;
    case Kind::UInt16:
      return 2;
    case Kind::UInt32:
      return 4;
    case Kind::UInt64:
      return 8;
    case Kind::Float32:
      return 4;
    case Kind::Float64:
      return 8;
    case Kind::Float80:
      abort();
    case Kind::Float128:
      return 16;
    case Kind::Bool:
      return 1;
    case Kind::UIntPtr:
      return sizeof(uintptr_t);
    case Kind::Void:
      return 0;
  }
}

} // namespace type_graph
