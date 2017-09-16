cmake_minimum_required(VERSION 3.6.2)
project(sdk-dslink-cpp)

include_directories(${PROJECT_INCLUDE_DIR})

# BUILD OPTIONS
option(INCLUDE_MODULE_PACK "Include set additional modules" OFF)

add_subdirectory("${PROJECT_SOURCE_DIR}/sdk")

if(INCLUDE_MODULE_PACK)
  add_subdirectory("${PROJECT_SOURCE_DIR}/module_pack")
endif()
