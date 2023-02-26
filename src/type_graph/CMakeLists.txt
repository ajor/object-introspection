add_library(type_graph
  AlignmentCalc.cpp
  DrgnParser.cpp
  NameGen.cpp
  Printer.cpp
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
