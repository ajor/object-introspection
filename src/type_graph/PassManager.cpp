#include "PassManager.h"

#include <iostream>

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
  Printer printer{std::cout};
  for (const auto &type : typeGraph.rootTypes()) {
    printer.print(type);
  }
}
} // namespace

const std::string separator = "----------------";

void PassManager::run(TypeGraph &typeGraph, bool debug) {
  if (debug) {
    std::cout << separator << std::endl;
    std::cout << "Parsed Type Graph:" << std::endl;
    std::cout << separator << std::endl;
    print(typeGraph);
    std::cout << separator << std::endl;
  }

  for (size_t i=0; i<passes_.size(); i++) {
    auto& pass = passes_[i];
    pass.run(typeGraph);
    if (debug) {
      std::cout << "Running pass (" << i+1 << "/" << passes_.size() << "): " << pass.name() << std::endl;
      std::cout << separator << std::endl;
      print(typeGraph);
      std::cout << separator << std::endl;
    }
  }
}

} // namespace type_graph
