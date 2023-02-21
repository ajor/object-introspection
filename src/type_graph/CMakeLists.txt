add_library(type_graph
  drgn_parser.cpp
  NameGen.cpp
  printer.cpp
  required_type_collector.cpp
  topo_sorter.cpp
  flattener.cpp
  types.cpp
)
add_dependencies(type_graph libdrgn)
target_link_libraries(type_graph
  symbol_service

  "-L${DRGN_PATH}/.libs"
  drgn
)
