cmake_minimum_required(VERSION 3.7)
project(sdk-dslink-cpp)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../include CACHE INTERNAL "")

include_directories(${PROJECT_INCLUDE_DIR})

# BUILD OPTIONS
option(INCLUDE_MODULE_PACK "Include set additional modules" OFF)

add_subdirectory("${PROJECT_SOURCE_DIR}/sdk")

if(INCLUDE_MODULE_PACK)
  add_subdirectory("${PROJECT_SOURCE_DIR}/module_pack")
endif()