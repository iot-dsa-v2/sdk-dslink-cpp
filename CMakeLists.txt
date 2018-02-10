cmake_minimum_required(VERSION 3.6.2)

project(DSA_SDK)

if (${CMAKE_SOURCE_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
  set( DSA_INCLUDE_TEST true )
  include(${CMAKE_CURRENT_LIST_DIR}/CMakeListsConfig.txt)
endif ()

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
