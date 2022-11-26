add_library(peddle STATIC

Peddle.cpp
PeddleDebugger.cpp

)

target_include_directories(peddle PUBLIC

${CMAKE_SOURCE_DIR}/.
${CMAKE_SOURCE_DIR}/Utilities
${CMAKE_SOURCE_DIR}/Peddle
)
