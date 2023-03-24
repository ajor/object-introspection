#include <gtest/gtest.h>

#include "ContainerInfo.h"
#include "type_graph/TypeIdentifier.h"

using namespace type_graph;

void test() {
  // TODO more container types, with various template parameter options
  ContainerInfo std_vector;
  std_vector.typeName = "std::vector";
  std_vector.matcher = "^std::vector<";
  std_vector.ctype = SEQ_TYPE;
  std_vector.stubTemplateParams = {1};

  // TODO move container testing into new test_type_identifier
  std::vector<ContainerInfo> containers;
  containers.emplace_back(std::move(std_vector));
}
