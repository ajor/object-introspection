#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#define OI_TYPE_LIST \
  X(Class) \
  X(Container) \
  X(Enum) \
  X(Primitive) \
  X(Typedef) \
  X(Pointer) \
  X(Array)

namespace type_graph {

class Visitor;
#define DECLARE_ACCEPT void accept(Visitor &v) override;

// TODO delete copy and move ctors

// TODO type qualifiers are needed for some stuff?
class Type {
public:
  virtual ~Type() = default;
  virtual void accept(Visitor &v) = 0;

  // TODO don't always return a copy for name()
  virtual std::string name() const = 0;
  virtual std::size_t size() const = 0;
  virtual uint64_t align() const = 0;
};

struct Member {
  Member(Type *type, const std::string &name, uint64_t offset)
    : type(type), name(name), offset(offset) { }

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

  Class(Kind kind, const std::string &name, std::size_t size)
    : kind_(kind), name_(name), size_(size) {
  }

  DECLARE_ACCEPT

  Kind kind() const {
    return kind_;
  }

  // TODO don't recalculate this on every call - save it in the ctor
  virtual std::string name() const override {
    return name_;
  }

  void setName(std::string name) {
    name_ = std::move(name);
  }

  virtual std::size_t size() const override {
    return size_;
  }

  virtual uint64_t align() const override {
    return align_;
  }

  void setAlign(uint64_t alignment) {
    align_ = alignment;
  }

  std::vector<TemplateParam> template_params;
  std::vector<Parent> parents; // Sorted by offset
  std::vector<Member> members; // Sorted by offset
  std::vector<Function> functions;

private:
  Kind kind_;
  std::string name_;
  std::size_t size_;
  uint64_t align_ = 0;
};

class Container : public Type {
public:
  enum class Kind {
    None,
    StdVector,
    StdMap,
  };

  Container(Kind kind) : kind_(kind) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  void setName(std::string name) {
    name_ = std::move(name);
  }

  virtual std::size_t size() const override {
    return 0; // TODO
  }

  virtual uint64_t align() const override {
    return 8; // TODO not needed for containers?
  }

  std::vector<TemplateParam> template_params;
  Kind kind_;

private:
  std::string name_ = "std::vector"; // TODO
};

class Enum : public Type {
public:
  explicit Enum(const std::string &name, std::size_t size)
    : name_(name), size_(size) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  virtual std::size_t size() const override {
    return size_;
  }

  virtual uint64_t align() const override {
    return size();
  }
private:
  std::string name_;
  std::size_t size_;
};

class Array : public Type {
public:
  Array(const std::string &name, std::size_t len, Type *element_type)
    : name_(name), len_(len), element_type_(element_type) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  virtual std::size_t size() const override {
    return len_ * element_type_->size();
  }

  virtual uint64_t align() const override {
    return element_type_->size();
  }

  Type *element_type() const {
    return element_type_;
  }

  // TODO remove std:: prefix from size_t
  std::size_t len() const {
    return len_;
  }

private:
  std::string name_;
  std::size_t len_;
  Type *element_type_;
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
  };

  explicit Primitive(Kind kind) : kind_(kind) { }

  DECLARE_ACCEPT

  virtual std::string name() const override;
  virtual std::size_t size() const override;
  virtual uint64_t align() const override {
    return size();
  }

private:
  Kind kind_;
};

class Typedef : public Type {
public:
  explicit Typedef(const std::string &name, Type *underlying_type)
    : name_(name), underlying_type_(underlying_type) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return name_;
  }

  virtual std::size_t size() const override {
    return underlying_type_->size();
  }

  virtual uint64_t align() const override {
    return underlying_type_->align();
  }

  Type *underlying_type() const {
    return underlying_type_;
  }

private:
  std::string name_;
  Type *underlying_type_;
};

class Pointer : public Type {
public:
  explicit Pointer(Type *pointee_type)
    : pointee_type_(pointee_type) { }

  DECLARE_ACCEPT

  virtual std::string name() const override {
    return ""; // TODO
  }

  virtual std::size_t size() const override {
    return sizeof(uintptr_t);
  }

  virtual uint64_t align() const override {
    return size();
  }

  Type *pointee_type() const {
    return pointee_type_;
  }

private:
  Type *pointee_type_;
};

} // namespace type_graph

#undef DECLARE_ACCEPT
