#include <gtest/gtest.h>

#include "ContainerInfo.h"
#include "type_graph/NameGen.h"
#include "type_graph/Types.h"

using namespace type_graph;

ContainerInfo vectorInfo() {
  ContainerInfo info;
  info.typeName = "std::vector";
  return info;
}

TEST(NameGenTest, ClassParams) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Struct, "MyClass", 13);
  myclass->templateParams.push_back(myparam1.get());
  myclass->templateParams.push_back(myparam2.get());

  NameGen nameGen;
  nameGen.generateNames({*myclass});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(myclass->name(), "MyClass_MyParam_0_MyParam_1_2");
}

TEST(NameGenTest, ClassContainerParam) {
  auto myint = std::make_unique<Primitive>(Primitive::Kind::Int32);
  auto myparam = std::make_unique<Container>(vectorInfo());
  myparam->templateParams.push_back(myint.get());

  auto myclass = std::make_unique<Class>(Class::Kind::Struct, "MyClass", 13);
  myclass->templateParams.push_back(myparam.get());

  NameGen nameGen;
  nameGen.generateNames({*myclass});

  EXPECT_EQ(myparam->name(), "std::vector<int32_t>");
  EXPECT_EQ(myclass->name(), "MyClass_std__vector_int32_t__0");
}

TEST(NameGenTest, ClassParents) {
  auto myparent1 = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myparent2 = std::make_unique<Class>(Class::Kind::Struct, "MyParent", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Struct, "MyClass", 13);
  myclass->parents.push_back(Parent{myparent1.get(), 0});
  myclass->parents.push_back(Parent{myparent2.get(), 0});

  NameGen nameGen;
  nameGen.generateNames({*myclass});

  EXPECT_EQ(myparent1->name(), "MyParent_0");
  EXPECT_EQ(myparent2->name(), "MyParent_1");
  EXPECT_EQ(myclass->name(), "MyClass_2");
}

TEST(NameGenTest, ClassMembers) {
  auto mymember1 = std::make_unique<Class>(Class::Kind::Struct, "MyMember", 13);
  auto mymember2 = std::make_unique<Class>(Class::Kind::Struct, "MyMember", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Struct, "MyClass", 13);
  myclass->members.push_back(Member{mymember1.get(), "mem1", 0});
  myclass->members.push_back(Member{mymember2.get(), "mem2", 0});

  NameGen nameGen;
  nameGen.generateNames({*myclass});

  EXPECT_EQ(mymember1->name(), "MyMember_0");
  EXPECT_EQ(mymember2->name(), "MyMember_1");
  EXPECT_EQ(myclass->name(), "MyClass_2");
}

TEST(NameGenTest, ClassMembersDuplicateName) {
  auto mymember1 = std::make_unique<Class>(Class::Kind::Struct, "MyMember", 13);
  auto mymember2 = std::make_unique<Class>(Class::Kind::Struct, "MyMember", 13);
  auto myclass = std::make_unique<Class>(Class::Kind::Struct, "MyClass", 13);

  // A class may end up with members sharing a name after flattening
  myclass->members.push_back(Member{mymember1.get(), "mem", 0});
  myclass->members.push_back(Member{mymember2.get(), "mem", 0});

  NameGen nameGen;
  nameGen.generateNames({*myclass});

  EXPECT_EQ(mymember1->name(), "MyMember_0");
  EXPECT_EQ(mymember2->name(), "MyMember_1");
  EXPECT_EQ(myclass->members[0].name, "mem_0");
  EXPECT_EQ(myclass->members[1].name, "mem_1");
  EXPECT_EQ(myclass->name(), "MyClass_2");
}

TEST(NameGenTest, ContainerParams) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam1.get());
  mycontainer->templateParams.push_back(myparam2.get());

  NameGen nameGen;
  nameGen.generateNames({*mycontainer});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
}

TEST(NameGenTest, ContainerParamsDuplicates) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam.get());
  mycontainer->templateParams.push_back(myparam.get());

  NameGen nameGen;
  nameGen.generateNames({*mycontainer});

  EXPECT_EQ(myparam->name(), "MyParam_0");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_0>");
}

TEST(NameGenTest, ContainerParamsDuplicatesDeep) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer1 = std::make_unique<Container>(vectorInfo());
  mycontainer1->templateParams.push_back(myparam.get());

  auto mycontainer2 = std::make_unique<Container>(vectorInfo());
  mycontainer2->templateParams.push_back(myparam.get());
  mycontainer2->templateParams.push_back(mycontainer1.get());

  NameGen nameGen;
  nameGen.generateNames({*mycontainer2});

  EXPECT_EQ(myparam->name(), "MyParam_0");
  EXPECT_EQ(mycontainer1->name(), "std::vector<MyParam_0>");
  EXPECT_EQ(mycontainer2->name(), "std::vector<MyParam_0, std::vector<MyParam_0>>");
}

TEST(NameGenTest, ContainerParamsDuplicatesAcrossContainers) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam3 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer1 = std::make_unique<Container>(vectorInfo());
  mycontainer1->templateParams.push_back(myparam1.get());
  mycontainer1->templateParams.push_back(myparam2.get());

  auto mycontainer2 = std::make_unique<Container>(vectorInfo());
  mycontainer2->templateParams.push_back(myparam2.get());
  mycontainer2->templateParams.push_back(myparam3.get());

  NameGen nameGen;
  nameGen.generateNames({*mycontainer1, *mycontainer2});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(myparam3->name(), "MyParam_2");
  EXPECT_EQ(mycontainer1->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(mycontainer2->name(), "std::vector<MyParam_1, MyParam_2>");
}

TEST(NameGenTest, Array) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam1.get());
  mycontainer->templateParams.push_back(myparam2.get());

  auto myarray = std::make_unique<Array>(mycontainer.get(), 5);

  NameGen nameGen;
  nameGen.generateNames({*myarray});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(myarray->name(), "std::vector<MyParam_0, MyParam_1>[5]");
}

TEST(NameGenTest, Typedef) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam1.get());
  mycontainer->templateParams.push_back(myparam2.get());

  auto mytypedef = std::make_unique<Typedef>("MyTypedef", mycontainer.get());

  NameGen nameGen;
  nameGen.generateNames({*mytypedef});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(mytypedef->name(), "MyTypedef");
}

TEST(NameGenTest, Pointer) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam1.get());
  mycontainer->templateParams.push_back(myparam2.get());

  auto mypointer = std::make_unique<Pointer>(mycontainer.get());

  NameGen nameGen;
  nameGen.generateNames({*mypointer});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(mypointer->name(), "std::vector<MyParam_0, MyParam_1>*");
}

TEST(NameGenTest, DummyAllocator) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back(myparam1.get());
  mycontainer->templateParams.push_back(myparam2.get());

  auto myalloc = std::make_unique<DummyAllocator>(*mycontainer.get(), 0, 0);

  NameGen nameGen;
  nameGen.generateNames({*myalloc});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(myalloc->name(), "std::allocator<std::vector<MyParam_0, MyParam_1>>");
}
