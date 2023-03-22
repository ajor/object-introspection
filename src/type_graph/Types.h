#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "ContainerInfo.h"

// TODO make all visitors and tests have classes in this order:
#define OI_TYPE_LIST \
  X(Class) \
  X(Container) \
  X(Primitive) \
  X(Enum) \
  X(Array) \
  X(Typedef) \
  X(Pointer) \
  X(Dummy)

struct ContainerInfo;

namespace type_graph {

class Visitor;
#define DECLARE_ACCEPT void accept(Visitor &v) override;

// TODO delete copy and move ctors

// TODO make types hold references instead of pointers
// TODO type qualifiers are needed for some stuff?
class Type {
public:
  virtual ~Type() = default;
  virtual void accept(Visitor &v) = 0;

  // TODO don't always return a copy for name()
  virtual std::string name() const = 0;
  virtual size_t size() const = 0;
  virtual uint64_t align() const = 0;
};

struct Member {
  Member(Type *type, const std::string &name, uint64_t offset, uint64_t align=0)
    : type(type), name(name), offset(offset), align(align) { }

  Type *type;
  std::string name; // TODO make optional?
  uint64_t offset;
  uint64_t align = 0;
};

struct Function {
  Function(const std::string &name, int virtuality=0)
    : name(name), virtuality(virtuality) { }

  std::string name;
  int virtuality;
};

class Class;
struct Parent {
  Parent(Type *type, uint64_t offset)
    : type(type), offset(offset) { }

  Type *type;
  uint64_t offset;
};

struct TemplateParam {
  TemplateParam(Type *type) : type(type) { }
  TemplateParam(Type *type, int value) : type(type), value(value) { }

  Type *type;
  std::optional<int> value; // TODO non-int values
};

class Class : public Type {
public:
  enum class Kind {
    Class,
    Struct,
    Union,
  };

  Class(Kind kind, const std::string &name, size_t size)
    : kind_(kind), name_(name), size_(size) {
  }

  DECLARE_ACCEPT

  Kind kind() const {
    return kind_;
  }

  virtual std::string name() const override {
    return name_;
  }

  void setName(std::string name) {
    name_ = std::move(name);
  }

  virtual size_t size() const override {
    return size_;
  }

  virtual uint64_t align() const override {
    return align_;
  }

  void setAlign(uint64_t alignment) {
    align_ = alignment;
  }

  std::vector<TemplateParam> templateParams;
  std::vector<Parent> parents; // Sorted by offset
  std::vector<Member> members; // Sorted by offset
  std::vector<Function> functions;

private:
  Kind kind_;
  std::string name_;
  size_t size_;
  uint64_t align_ = 0;
};

class Container : public Type {
public:
  Container(const ContainerInfo &containerInfo) : containerInfo_(containerInfo), name_(containerInfo.typeName) { }

  DECLARE_ACCEPT

  const std::string &containerName() const {
    return containerInfo_.typeName;
  }

  virtual std::string name() const override {
    return name_;
  }

  void setName(std::string name) {
    name_ = std::move(name);
  }

  virtual size_t size() const override {
    return 0; // TODO
  }

  virtual uint64_t align() const override {
    return 8; // TODO not needed for containers?
  }

  std::vector<TemplateParam> templateParams;
  const ContainerInfo &containerInfo_;

private:
  std::string name_;
};

class Enum : public Type {
public:
  explicit Enum(const std::string &name, size_t size)
    : name_(name), size_(size) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  virtual size_t size() const override {
    return size_;
  }

  virtual uint64_t align() const override {
    return size();
  }
private:
  std::string name_;
  size_t size_;
};

class Array : public Type {
public:
  Array(Type *elementType, size_t len)
    : elementType_(elementType), len_(len) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return elementType_->name() + "[" + std::to_string(len_) + "]";
  }

  virtual size_t size() const override {
    return len_ * elementType_->size();
  }

  virtual uint64_t align() const override {
    return elementType_->size();
  }

  Type *elementType() const {
    return elementType_;
  }

  // TODO remove std:: prefix from size_t
  size_t len() const {
    return len_;
  }

private:
  Type *elementType_;
  size_t len_;
};

class Primitive : public Type {
public:
  enum class Kind {
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    Float80, // TODO worth including?
    Float128, // TODO can we generate this?
    Bool,

    UIntPtr, // Really an alias, but usful to have as it's own primitive
  };

  explicit Primitive(Kind kind) : kind_(kind) { }

  DECLARE_ACCEPT

  virtual std::string name() const override;
  virtual size_t size() const override;
  virtual uint64_t align() const override {
    return size();
  }

private:
  Kind kind_;
};

class Typedef : public Type {
public:
  explicit Typedef(const std::string &name, Type *underlyingType)
    : name_(name), underlyingType_(underlyingType) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  virtual size_t size() const override {
    return underlyingType_->size();
  }

  virtual uint64_t align() const override {
    return underlyingType_->align();
  }

  Type *underlyingType() const {
    return underlyingType_;
  }

private:
  std::string name_;
  Type *underlyingType_;
};

class Pointer : public Type {
public:
  explicit Pointer(Type *pointeeType)
    : pointeeType_(pointeeType) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return pointeeType_->name() + "*";
  }

  virtual size_t size() const override {
    return sizeof(uintptr_t);
  }

  virtual uint64_t align() const override {
    return size();
  }

  Type *pointeeType() const {
    return pointeeType_;
  }

private:
  Type *pointeeType_;
};

class Dummy : public Type {
public:
  explicit Dummy(size_t size, uint64_t align)
    : size_(size), align_(align) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    // TODO change name:
    return "std::allocator<int>";
//    return "DummyAllocator<int, " + std::to_string(size_) + "," + std::to_string(align_) + ">";
  }

  virtual size_t size() const override {
    return size_;
  }

  virtual uint64_t align() const override {
    return align_;
  }

private:
  size_t size_;
  uint64_t align_;
};

} // namespace type_graph

#undef DECLARE_ACCEPT
