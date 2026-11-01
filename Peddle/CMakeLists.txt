add_library(peddle STATIC

Peddle.cpp
PeddleDebugger.cpp
PeddleDisassembler.cpp
StrWriter.cpp
SymbolTable.cpp

)

target_include_directories(peddle PUBLIC

${CMAKE_SOURCE_DIR}/.
${CMAKE_SOURCE_DIR}/Peddle

)
