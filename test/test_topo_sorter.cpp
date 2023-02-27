#include <gtest/gtest.h>

#include "type_graph/TopoSorter.h"
#include "type_graph/Types.h"

using namespace type_graph;

template <typename T>
using ref = std::reference_wrapper<T>;

void EXPECT_EQ_TYPES(const std::vector<ref<Type>> expected,
                     const std::vector<ref<Type>> actual) {
  EXPECT_EQ(expected.size(), actual.size());
  for (size_t i=0; i<std::min(actual.size(), expected.size()); i++) {
    EXPECT_EQ(expected[i].get().name(), actual[i].get().name());
  }
}

TEST(TopoSorterTest, SingleType) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);

  std::vector<ref<Type>> input = {*myint};

  TopoSorter topo;
  EXPECT_EQ_TYPES(input, topo.sort(input));
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
    EXPECT_EQ_TYPES(input, topo.sort(input));
  }
}

TEST(TopoSorterTest, PrimitiveMembers) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myenum = std::make_unique<Enum>("MyEnum", 4);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(myint.get(), "n", 0));
  myclass->members.push_back(Member(myenum.get(), "e", 4));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*myint, *myenum, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, ClassMembers) {
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*mystruct, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, Parents) { // TODO there should be no parents at this point
  auto myparent = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->parents.push_back(Parent(myparent.get(), 0));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*myparent, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, TemplateParams) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->templateParams.push_back(TemplateParam(myparam.get()));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*myparam, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, Containers) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(SEQ_TYPE);
  mycontainer->templateParams.push_back((myparam.get()));

  std::vector<ref<Type>> input = {*mycontainer};
  std::vector<ref<Type>> expected = {*myparam, *mycontainer};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, Arrays) {
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto myarray = std::make_unique<Array>(myclass.get(), 10);

  std::vector<ref<Type>> input = {*myarray, *myclass};
  std::vector<ref<Type>> expected = {*myclass, *myarray};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, Typedef) {
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 8);

  auto aliasA = std::make_unique<Typedef>("aliasA", classA.get());

  std::vector<ref<Type>> input = {*aliasA};
  std::vector<ref<Type>> expected = {*classA, *aliasA};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, Pointers) {
  // Pointers do not require pointee types to be defined first
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  auto mypointer = std::make_unique<Pointer>(myclass.get());

  std::vector<ref<Type>> input = {*mypointer};
  std::vector<ref<Type>> expected = {*mypointer, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, PointerCycle) {
  auto classA = std::make_unique<Class>(Class::Kind::Class, "ClassA", 69);
  auto classB = std::make_unique<Class>(Class::Kind::Class, "ClassB", 69);
  auto ptrA = std::make_unique<Pointer>(classA.get());
  classA->members.push_back(Member(classB.get(), "b", 0));
  classB->members.push_back(Member(ptrA.get(), "a", 0));

  std::vector<std::vector<ref<Type>>> inputs = {
    {*classA},
    {*classB},
    {*ptrA},
  };
  std::vector<ref<Type>> expected = {*ptrA, *classB, *classA};

  for (const auto &input : inputs) {
    TopoSorter topo;
    EXPECT_EQ_TYPES(expected, topo.sort(input));
  }
}

TEST(TopoSorterTest, TwoDeep) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion", 0));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*myunion, *mystruct, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
}

TEST(TopoSorterTest, MultiplePaths) {
  auto myunion = std::make_unique<Class>(Class::Kind::Union, "MyUnion", 7);
  auto mystruct = std::make_unique<Class>(Class::Kind::Struct, "MyStruct", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Class, "MyClass", 69);
  myclass->members.push_back(Member(mystruct.get(), "mystruct", 0));
  myclass->members.push_back(Member(myunion.get(), "myunion1", 0));
  mystruct->members.push_back(Member(myunion.get(), "myunion2", 0));

  std::vector<ref<Type>> input = {*myclass};
  std::vector<ref<Type>> expected = {*myunion, *mystruct, *myclass};

  TopoSorter topo;
  EXPECT_EQ_TYPES(expected, topo.sort(input));
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
//    EXPECT_EQ_TYPES(expected, topo.sort(input));
//  }
//}

// TODO:
// References
// Typedefs
