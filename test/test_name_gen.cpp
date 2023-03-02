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

TEST(NameGenTest, ContainerParams) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back((myparam1.get()));
  mycontainer->templateParams.push_back((myparam2.get()));

  NameGen nameGen;
  nameGen.generateNames({*mycontainer});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
}

TEST(NameGenTest, ContainerParamsDuplicates) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(vectorInfo());
  mycontainer->templateParams.push_back((myparam.get()));
  mycontainer->templateParams.push_back((myparam.get()));

  NameGen nameGen;
  nameGen.generateNames({*mycontainer});

  EXPECT_EQ(myparam->name(), "MyParam_0");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_0>");
}

TEST(NameGenTest, ContainerParamsDuplicatesDeep) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer1 = std::make_unique<Container>(vectorInfo());
  mycontainer1->templateParams.push_back((myparam.get()));

  auto mycontainer2 = std::make_unique<Container>(vectorInfo());
  mycontainer2->templateParams.push_back((myparam.get()));
  mycontainer2->templateParams.push_back((mycontainer1.get()));

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
  mycontainer1->templateParams.push_back((myparam1.get()));
  mycontainer1->templateParams.push_back((myparam2.get()));

  auto mycontainer2 = std::make_unique<Container>(vectorInfo());
  mycontainer2->templateParams.push_back((myparam2.get()));
  mycontainer2->templateParams.push_back((myparam3.get()));

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
  mycontainer->templateParams.push_back((myparam1.get()));
  mycontainer->templateParams.push_back((myparam2.get()));

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
  mycontainer->templateParams.push_back((myparam1.get()));
  mycontainer->templateParams.push_back((myparam2.get()));

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
  mycontainer->templateParams.push_back((myparam1.get()));
  mycontainer->templateParams.push_back((myparam2.get()));

  auto mypointer = std::make_unique<Pointer>(mycontainer.get());

  NameGen nameGen;
  nameGen.generateNames({*mypointer});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(mypointer->name(), "std::vector<MyParam_0, MyParam_1>*");
}
