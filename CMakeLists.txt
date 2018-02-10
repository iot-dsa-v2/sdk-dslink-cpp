cmake_minimum_required(VERSION 3.6.2)

set( DSA_INCLUDE_TEST true )

project(DSA_SDK)


include(${CMAKE_CURRENT_LIST_DIR}/CMakeListsConfig.txt)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/msgpack-build")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/jansson-build")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/src")

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

if (DSA_INCLUDE_TEST)
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/test")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/benchmark")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/example")
  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/playground")
endif(DSA_INCLUDE_TEST)
