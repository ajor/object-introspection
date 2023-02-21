#include <gtest/gtest.h>

#include "type_graph/TopoSorter.h"
#include "type_graph/Types.h"

using namespace type_graph;

void EXPECT_EQ_TYPE(const Type *actual, const Type *expected) {
  EXPECT_EQ(actual->name(), expected->name());
  EXPECT_EQ(actual->size(), expected->size());
}

void EXPECT_EQ_TYPES(const std::vector<Type*> actual, const std::vector<Type*> expected) {
  EXPECT_EQ(actual.size(), expected.size());
  for (std::size_t i=0; i<std::min(actual.size(), expected.size()); i++) {
    EXPECT_EQ_TYPE(actual[i], expected[i]);
  }
}

TEST(TopoSorterTest, SingleType) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  std::vector<Type*> input = {myint.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), input);
}

TEST(TopoSorterTest, UnrelatedTypes) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myenum = std::make_unique<Enum>("MyEnum", 4);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);

  // Try the same input in a few different orders
  std::vector<std::vector<Type*>> inputs = {
    {myint.get(), myenum.get(), myclass.get()},
    {myenum.get(), myint.get(), myclass.get()},
    {myclass.get(), myenum.get(), myint.get()},
  };

  for (const auto &input : inputs) {
    TopoSorter topo;
    EXPECT_EQ_TYPES(topo.sort(input), input);
  }
}

TEST(TopoSorterTest, PrimitiveMembers) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myenum = std::make_unique<Enum>("MyEnum", 4);
  auto mybool = std::make_unique<Primitive>(Primitive::Kind::Bool);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(myint.get(), "n", 0));
  myclass->members.push_back(Member(myenum.get(), "e", 4));

  std::vector<Type*> input = {myclass.get(), myint.get(), myenum.get(), mybool.get()};
  std::vector<Type*> expected = {myint.get(), myenum.get(), myclass.get(), mybool.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, ClassMembers) {
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));

  std::vector<Type*> input = {myclass.get(), mystruct.get()};
  std::vector<Type*> expected = {mystruct.get(), myclass.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, Parents) {
  auto myparent = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->parents.push_back(Parent(myparent.get(), 0));

  std::vector<Type*> input = {myclass.get(), myparent.get()};
  std::vector<Type*> expected = {myparent.get(), myclass.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, TemplateParams) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->template_params.push_back(TemplateParam(myparam.get()));

  std::vector<Type*> input = {myclass.get(), myparam.get()};
  std::vector<Type*> expected = {myparam.get(), myclass.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, Containers) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(Container::Kind::StdVector);
  mycontainer->template_params.push_back((myparam.get()));

  std::vector<Type*> input = {mycontainer.get(), myparam.get()};
  std::vector<Type*> expected = {myparam.get(), mycontainer.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

//TEST(TopoSorterTest, ContainerInternalsIgnored) {
//  // Members and parents of container types are not important for us and should
//  // not be sorted along with template parameters
//  auto mymember = std::make_unique<Class>(Class::Kind::Struct, "MyMember", 13);
//  auto myparent = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
//  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
//  auto mycontainer = std::make_unique<Container>("MyContainer");
//  mycontainer->members.push_back(Member(mymember.get(), "mymember", 0));
//  mycontainer->parents.push_back(Parent(myparent.get(), 0));
//  mycontainer->template_params.push_back(TemplateParam(myparam.get()));
//
//  std::vector<Type*> input = {mycontainer.get(), mymember.get(), myparent.get(), myparam.get()};
//  std::vector<Type*> expected = {myparam.get(), mycontainer.get(), mymember.get(), myparent.get()};
//
//  TopoSorter topo;
//  EXPECT_EQ_TYPES(topo.sort(input), expected);
//}

TEST(TopoSorterTest, Pointers) {
  // Pointers do not introduce any ordering requirements
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto mypointer = std::make_unique<Pointer>(myclass.get());

  std::vector<Type*> input = {mypointer.get(), myclass.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), input);
}

TEST(TopoSorterTest, Arrays) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto myarray = std::make_unique<Array>("myarray", 10, myclass.get());

  std::vector<Type*> input = {myarray.get(), myclass.get()};
  std::vector<Type*> expected = {myclass.get(), myarray.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, TwoDeep) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion", 0));

  std::vector<Type*> input = {myclass.get(), mystruct.get(), myunion.get()};
  std::vector<Type*> expected = {myunion.get(), mystruct.get(), myclass.get()};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, MultiplePaths) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  myclass->members.push_back(Member(myunion.get(), "myunion1", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion2", 0));

  std::vector<std::vector<Type*>> inputs = {
    {myclass.get(), mystruct.get(), myunion.get()},
    {myclass.get(), myunion.get(), mystruct.get()},
    {mystruct.get(), myunion.get(), myclass.get()},
  };
  std::vector<Type*> expected = {myunion.get(), mystruct.get(), myclass.get()};

  for (const auto &input : inputs) {
    TopoSorter topo;
    EXPECT_EQ_TYPES(topo.sort(input), expected);
  }
}

TEST(TopoSorterTest, PointerCycle) {
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
  auto ptrA = std::make_unique<Pointer>(classA.get());
  classA->members.push_back(Member(classB.get(), "b", 0));
  classB->members.push_back(Member(ptrA.get(), "a", 0));

  std::vector<std::vector<Type*>> inputs = {
    {classA.get(), classB.get(), ptrA.get()},
    {classA.get(), ptrA.get(), classB.get()},
    {classB.get(), classA.get(), ptrA.get()},
    {classB.get(), ptrA.get(), classA.get()},
    {ptrA.get(), classA.get(), classB.get()},
    {ptrA.get(), classB.get(), classA.get()},
  };
  std::vector<Type*> expected = {ptrA.get(), classB.get(), classA.get()};

  for (const auto &input : inputs) {
    TopoSorter topo;
    EXPECT_EQ_TYPES(topo.sort(input), expected);
  }
}

//TEST(TopoSorterTest, BigTest) {
//  /*
//   *   A      B
//   *  / \   /  \
//   * v   v v    v
//   * C    i     e
//   *  \        ^
//   *   \      /
//   *    > arr
//   */
//  auto i = std::make_unique<Primitive>(Primitive::Kind::Int32);
//  auto e = std::make_unique<Enum>("MyEnum", 4);
//  auto A = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
//  auto B = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
//  auto C = std::make_unique<Class>(Class::Kind::Class, "ClassC", 69);
//  auto pA = std::make_unique<Pointer>(myclass.get());
//  auto pB = std::make_unique<Pointer>(myclass.get());
//  auto pC = std::make_unique<Pointer>(myclass.get());
//
//  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
//  myclass->members.push_back(Member(myunion.get(), "myunion1", 0));
//  mystruct->members.push_back(Member(myunion.get(), "myunion2", 0));
//
//  std::vector<std::vector<Type*>> inputs = {
//    {myclass.get(), mystruct.get(), myunion.get()},
//    {myclass.get(), myunion.get(), mystruct.get()},
//    {mystruct.get(), myunion.get(), myclass.get()},
//  };
//  std::vector<Type*> expected = {myunion.get(), mystruct.get(), myclass.get()};
//
//  for (const auto &input : inputs) {
//    TopoSorter topo;
//    EXPECT_EQ_TYPES(topo.sort(input), expected);
//  }
//}

// TODO:
// References
// Typedefs
