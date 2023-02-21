add_library(type_graph
  DrgnParser.cpp
  NameGen.cpp
  Printer.cpp
  RequiredTypeCollector.cpp
  TopoSorter.cpp
  Flattener.cpp
  Types.cpp
)
add_dependencies(type_graph libdrgn)
target_link_libraries(type_graph
  symbol_service

  "-L${DRGN_PATH}/.libs"
  drgn
)
