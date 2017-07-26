cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include CACHE INTERNAL "")

add_subdirectory("./src")
add_subdirectory("./test")
#add_subdirectory("./example")
include_directories("./include")
