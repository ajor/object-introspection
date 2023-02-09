#include <gtest/gtest.h>

#include "required_type_collector.h"
#include "types.h"

void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected);

TEST(RequiredTypeCollectorTest, Primitives) {
  // Primitives don't need to be defined
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  std::vector<Type*> input = {myint.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), {});
}

TEST(RequiredTypeCollectorTest, Enums) {
  auto myenum = std::make_unique<Enum>("MyEnum", 4);

  std::vector<Type*> input = {myenum.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, Classes) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);

  std::vector<Type*> input = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, ArrayOfClasses) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto myarray = std::make_unique<Array>("myarray", 10, myclass.get());

  std::vector<Type*> input = {myarray.get()};
  std::vector<Type*> expected = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), expected);
}

TEST(RequiredTypeCollectorTest, ArrayOfPointers) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto myptr = std::make_unique<Pointer>(myclass.get());
  auto myarray = std::make_unique<Array>("myarray", 10, myptr.get());

  std::vector<Type*> input = {myarray.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), {});
}

TEST(RequiredTypeCollectorTest, PrimitiveMembers) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(myint.get(), "n", 0));

  std::vector<Type*> input = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, ClassMembers) {
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));

  std::vector<Type*> input = {myclass.get()};
  std::vector<Type*> expected = {myclass.get(), mystruct.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), expected);
}

TEST(RequiredTypeCollectorTest, Parents) {
  auto myparent = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->parents.push_back(Parent(myparent.get(), 0));

  std::vector<Type*> input = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, TemplateParams) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->template_params.push_back(TemplateParam(myparam.get()));

  std::vector<Type*> input = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, ContainerMembers) {
  auto mycontainer = std::make_unique<Container>("MyContainer");
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mycontainer.get(), "mycontainer", 0));

  std::vector<Type*> input = {myclass.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), input);
}

TEST(RequiredTypeCollectorTest, ContainerParams) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>("MyContainer");
  mycontainer->template_params.push_back((myparam.get()));

  std::vector<Type*> input = {mycontainer.get()};
  std::vector<Type*> expected = {myparam.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), expected);
}

TEST(RequiredTypeCollectorTest, TwoDeep) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion", 0));

  std::vector<Type*> input = {myclass.get()};
  std::vector<Type*> expected = {myclass.get(), mystruct.get(), myunion.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), expected);
}

TEST(RequiredTypeCollectorTest, MultiplePaths) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  myclass->members.push_back(Member(myunion.get(), "myunion1", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion2", 0));

  std::vector<Type*> input = {myclass.get()};
  std::vector<Type*> expected = {myclass.get(), mystruct.get(), myunion.get()};

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect(input), expected);
}

TEST(RequiredTypeCollectorTest, PointerCycle) {
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
  auto ptrA = std::make_unique<Pointer>(classA.get());
  classA->members.push_back(Member(classB.get(), "b", 0));
  classB->members.push_back(Member(ptrA.get(), "a", 0));

  {
    RequiredTypeCollector req_types;
    EXPECT_EQ_TYPES(req_types.collect({classA.get()}), {classA.get(), classB.get()});
  }
  {
    RequiredTypeCollector req_types;
    EXPECT_EQ_TYPES(req_types.collect({classB.get()}), {classB.get()});
  }
}

TEST(RequiredTypeCollectorTest, TopLevelPointer) {
  // Top-level pointer should be followed
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
  auto ptrA = std::make_unique<Pointer>(classA.get());

  // Non-top-level pointer should not be followed
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
  auto ptrB = std::make_unique<Pointer>(classB.get());
  classA->members.push_back(Member(ptrB.get(), "b", 0));

  RequiredTypeCollector req_types;
  EXPECT_EQ_TYPES(req_types.collect({ptrA.get()}), {classA.get()});
}

// TODO:
// References
// Typedefs
