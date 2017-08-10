cmake_minimum_required(VERSION 2.7)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CMakeHelpers)

use_cxx11()

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_subdirectory("./src")
add_subdirectory("./test")
#add_subdirectory("./example")
include_directories("./include")
