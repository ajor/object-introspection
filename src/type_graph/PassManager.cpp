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

void PassManager::run(TypeGraph &typeGraph, bool debug) {
  print(typeGraph);
  for (auto &pass : passes_) {
    pass.run(typeGraph);
    if (debug) {
      std::cout << "Running pass: " << pass.name() << std::endl;
      print(typeGraph);
    }
  }
}

} // namespace type_graph
