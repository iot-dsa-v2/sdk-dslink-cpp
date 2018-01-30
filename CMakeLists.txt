cmake_minimum_required(VERSION 3.6.2)

project(DSA_SDK)

include(CMakeListsConfig.txt)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/msgpack-build")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/jansson-build")
add_subdirectory("./src")

include_directories("./include")


if (DSA_INCLUDE_TEST)
  add_subdirectory("./test")
  add_subdirectory("./benchmark")
  add_subdirectory("./example")
  add_subdirectory("./playground")
endif(DSA_INCLUDE_TEST)


