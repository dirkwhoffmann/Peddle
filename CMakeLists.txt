cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# Set the project name
project(main)

# Enable testing
enable_testing()

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Add sub directories
add_subdirectory(Peddle)

# Add the executable
add_executable(main main.cpp)
target_link_libraries(main peddle)

# Add compile options
if(MSVC)
  target_compile_options(main PUBLIC /wd4100 /wd4201 /wd4324 /wd4458)
else()
  target_compile_options(main PUBLIC -Wno-unused-parameter)
  target_compile_options(main PUBLIC -Wno-unused-variable)
endif()

# Add include paths
target_include_directories(main PUBLIC

${CMAKE_SOURCE_DIR}/.
${CMAKE_SOURCE_DIR}/Utilities
${CMAKE_SOURCE_DIR}/Peddle
)

# Add tests
add_test(UnitTest main)

