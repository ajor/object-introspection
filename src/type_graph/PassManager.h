#pragma once

#include <functional>
#include <string>
#include <vector>

namespace type_graph {

class TypeGraph;
class Type;

/*
 * Pass
 *
 * TODO
 */
class Pass {
  using PassFn = std::function<void(TypeGraph &typeGraph)>;

public:
  Pass(std::string name, PassFn fn) : name_(std::move(name)), fn_(fn) { }
  void run(TypeGraph &typeGraph);

private:
  std::string name_;
  PassFn fn_;
};

/*
 * PassManager
 *
 * TODO
 */
class PassManager {
public:
  void addPass(Pass p);
  void run(TypeGraph &typeGraph);

private:
  std::vector<Pass> passes_;
};

} // namespace type_graph
