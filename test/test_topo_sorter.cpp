#include <gtest/gtest.h>

#include "type_graph/TopoSorter.h"
#include "type_graph/Types.h"

using namespace type_graph;

template <typename T>
using ref = std::reference_wrapper<T>;

void EXPECT_EQ_TYPE(const Type &actual, const Type &expected) {
  EXPECT_EQ(actual.name(), expected.name());
  EXPECT_EQ(actual.size(), expected.size());
}

void EXPECT_EQ_TYPES(const std::vector<ref<Type>> actual,
                     const std::vector<ref<Type>> expected) {
  EXPECT_EQ(actual.size(), expected.size());
  for (std::size_t i=0; i<std::min(actual.size(), expected.size()); i++) {
    EXPECT_EQ_TYPE(actual[i], expected[i]);
  }
}

TEST(TopoSorterTest, SingleType) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  std::vector<ref<Type>> input = {*myint};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), input);
}

TEST(TopoSorterTest, UnrelatedTypes) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myenum = std::make_unique<Enum>("MyEnum", 4);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);

  // Try the same input in a few different orders
  std::vector<std::vector<ref<Type>>> inputs = {
    {*myint, *myenum, *myclass},
    {*myenum, *myint, *myclass},
    {*myclass, *myenum, *myint},
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

  std::vector<ref<Type>> input = {*myclass, *myint, *myenum, *mybool};
  std::vector<ref<Type>> expected = {*myint, *myenum, *myclass, *mybool};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, ClassMembers) {
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));

  std::vector<ref<Type>> input = {*myclass, *mystruct};
  std::vector<ref<Type>> expected = {*mystruct, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, Parents) {
  auto myparent = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->parents.push_back(Parent(myparent.get(), 0));

  std::vector<ref<Type>> input = {*myclass, *myparent};
  std::vector<ref<Type>> expected = {*myparent, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, TemplateParams) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->templateParams.push_back(TemplateParam(myparam.get()));

  std::vector<ref<Type>> input = {*myclass, *myparam};
  std::vector<ref<Type>> expected = {*myparam, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, Containers) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(SEQ_TYPE);
  mycontainer->templateParams.push_back((myparam.get()));

  std::vector<ref<Type>> input = {*mycontainer, *myparam};
  std::vector<ref<Type>> expected = {*myparam, *mycontainer};

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
//  mycontainer->templateParams.push_back(TemplateParam(myparam.get()));
//
//  std::vector<ref<Type>> input = {*mycontainer, *mymember, *myparent, *myparam};
//  std::vector<ref<Type>> expected = {*myparam, *mycontainer, *mymember, *myparent};
//
//  TopoSorter topo;
//  EXPECT_EQ_TYPES(topo.sort(input), expected);
//}

TEST(TopoSorterTest, Pointers) {
  // Pointers do not introduce any ordering requirements
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto mypointer = std::make_unique<Pointer>(myclass.get());

  std::vector<ref<Type>> input = {*mypointer, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), input);
}

TEST(TopoSorterTest, Arrays) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto myarray = std::make_unique<Array>(myclass.get(), 10);

  std::vector<ref<Type>> input = {*myarray, *myclass};
  std::vector<ref<Type>> expected = {*myclass, *myarray};

  TopoSorter topo;
  EXPECT_EQ_TYPES(topo.sort(input), expected);
}

TEST(TopoSorterTest, TwoDeep) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion", 0));

  std::vector<ref<Type>> input = {*myclass, *mystruct, *myunion};
  std::vector<ref<Type>> expected = {*myunion, *mystruct, *myclass};

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

  std::vector<std::vector<ref<Type>>> inputs = {
    {*myclass, *mystruct, *myunion},
    {*myclass, *myunion, *mystruct},
    {*mystruct, *myunion, *myclass},
  };
  std::vector<ref<Type>> expected = {*myunion, *mystruct, *myclass};

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

  std::vector<std::vector<ref<Type>>> inputs = {
    {*classA, *classB, *ptrA},
    {*classA, *ptrA, *classB},
    {*classB, *classA, *ptrA},
    {*classB, *ptrA, *classA},
    {*ptrA, *classA, *classB},
    {*ptrA, *classB, *classA},
  };
  std::vector<ref<Type>> expected = {*ptrA, *classB, *classA};

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
//  std::vector<std::vector<ref<Type>>> inputs = {
//    {*myclass, *mystruct, *myunion},
//    {*myclass, *myunion, *mystruct},
//    {*mystruct, *myunion, *myclass},
//  };
//  std::vector<ref<Type>> expected = {*myunion, *mystruct, *myclass};
//
//  for (const auto &input : inputs) {
//    TopoSorter topo;
//    EXPECT_EQ_TYPES(topo.sort(input), expected);
//  }
//}

// TODO:
// References
// Typedefs
