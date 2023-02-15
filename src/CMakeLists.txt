add_library(symbol_service SymbolService.cpp)
target_link_libraries(symbol_service
  "-L${DRGN_PATH}/.libs"
  drgn
  dw
  glog::glog
)

add_library(codegen OICodeGen.cpp)
target_link_libraries(codegen symbol_service)
