#include <gtest/gtest.h>

#include "type_graph/NameGen.h"
#include "type_graph/Types.h"

#include "ContainerInfo.h"

using namespace type_graph;

TEST(NameGenTest, ContainerParams) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(SEQ_TYPE);
  mycontainer->templateParams.push_back((myparam1.get()));
  mycontainer->templateParams.push_back((myparam2.get()));

  NameGen nameGen;
  nameGen.generateNames({mycontainer.get()});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_1>");
}

TEST(NameGenTest, ContainerParamsDuplicates) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto mycontainer = std::make_unique<Container>(SEQ_TYPE);
  mycontainer->templateParams.push_back((myparam.get()));
  mycontainer->templateParams.push_back((myparam.get()));

  NameGen nameGen;
  nameGen.generateNames({mycontainer.get()});

  EXPECT_EQ(myparam->name(), "MyParam_0");
  EXPECT_EQ(mycontainer->name(), "std::vector<MyParam_0, MyParam_0>");
}

TEST(NameGenTest, ContainerParamsDuplicatesDeep) {
  auto myparam = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer1 = std::make_unique<Container>(SEQ_TYPE);
  mycontainer1->templateParams.push_back((myparam.get()));

  auto mycontainer2 = std::make_unique<Container>(SEQ_TYPE);
  mycontainer2->templateParams.push_back((myparam.get()));
  mycontainer2->templateParams.push_back((mycontainer1.get()));

  NameGen nameGen;
  nameGen.generateNames({mycontainer2.get()});

  EXPECT_EQ(myparam->name(), "MyParam_0");
  EXPECT_EQ(mycontainer1->name(), "std::vector<MyParam_0>");
  EXPECT_EQ(mycontainer2->name(), "std::vector<MyParam_0, std::vector<MyParam_0>>");
}

TEST(NameGenTest, ContainerParamsDuplicatesAcrossContainers) {
  auto myparam1 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam2 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);
  auto myparam3 = std::make_unique<Class>(Class::Kind::Struct, "MyParam", 13);

  auto mycontainer1 = std::make_unique<Container>(SEQ_TYPE);
  mycontainer1->templateParams.push_back((myparam1.get()));
  mycontainer1->templateParams.push_back((myparam2.get()));

  auto mycontainer2 = std::make_unique<Container>(SEQ_TYPE);
  mycontainer2->templateParams.push_back((myparam2.get()));
  mycontainer2->templateParams.push_back((myparam3.get()));

  NameGen nameGen;
  nameGen.generateNames({mycontainer1.get(), mycontainer2.get()});

  EXPECT_EQ(myparam1->name(), "MyParam_0");
  EXPECT_EQ(myparam2->name(), "MyParam_1");
  EXPECT_EQ(myparam3->name(), "MyParam_2");
  EXPECT_EQ(mycontainer1->name(), "std::vector<MyParam_0, MyParam_1>");
  EXPECT_EQ(mycontainer2->name(), "std::vector<MyParam_1, MyParam_2>");
}
