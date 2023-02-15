add_library(drgn_helpers drgn_helpers.cpp)
target_link_libraries(drgn_helpers
  "-L${DRGN_PATH}/.libs"
  drgn
)

add_library(symbol_service
  Descs.cpp
  SymbolService.cpp
)
target_link_libraries(symbol_service
  drgn_helpers
  dw
  glog::glog
)

add_library(codegen OICodeGen.cpp)
target_link_libraries(codegen symbol_service)
