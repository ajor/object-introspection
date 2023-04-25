#include "PassManager.h"

#include <sstream>

#include <glog/logging.h>

#include "Printer.h"
#include "TypeGraph.h"

template <typename T>
using ref = std::reference_wrapper<T>;

namespace type_graph {

void Pass::run(TypeGraph &typeGraph) {
  fn_(typeGraph);
}

void PassManager::addPass(Pass p) {
  passes_.push_back(std::move(p));
}

namespace {
void print(const TypeGraph &typeGraph) {
  if (!VLOG_IS_ON(1))
    return;

  // TODO: Long strings will be truncated by glog. Find another way to do this
  std::stringstream out;
  Printer printer{out};
  for (const auto &type : typeGraph.rootTypes()) {
    printer.print(type);
  }

  LOG(INFO) << "\n" << out.str();
}
} // namespace

const std::string separator = "----------------";

void PassManager::run(TypeGraph &typeGraph) {
  VLOG(1) << separator;
  VLOG(1) << "Parsed Type Graph:";
  VLOG(1) << separator;
  print(typeGraph);
  VLOG(1) << separator;

  for (size_t i=0; i<passes_.size(); i++) {
    auto& pass = passes_[i];
    LOG(INFO) << "Running pass (" << i+1 << "/" << passes_.size() << "): " << pass.name();
    pass.run(typeGraph);
    VLOG(1) << separator;
    print(typeGraph);
    VLOG(1) << separator;
  }
}

} // namespace type_graph
