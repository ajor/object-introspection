add_library(type_graph
  AlignmentCalc.cpp
  DrgnParser.cpp
  NameGen.cpp
  PassManager.cpp
  Printer.cpp
  TopoSorter.cpp
  Flattener.cpp
  RemoveTopLevelPointer.cpp
  TypeIdentifier.cpp
  Types.cpp
)
add_dependencies(type_graph libdrgn)
target_link_libraries(type_graph
  symbol_service

  "-L${DRGN_PATH}/.libs"
  drgn
)
