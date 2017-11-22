cmake_minimum_required(VERSION 3.6.2)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif ()

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)
    set(CMAKE_CXX_STANDARD 14)
else ()
    set(CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDSA_DEBUG")

if (CMAKE_COMPILER_IS_GNUCC AND NOT CYGWIN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
endif()

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

option(INCLUDE_BROKER "build dsa broker" ON)

option(INCLUDE_MODULE_PACK "Include set additional modules" OFF)

add_subdirectory("${CMAKE_SOURCE_DIR}/deps/msgpack-build")
add_subdirectory("${CMAKE_SOURCE_DIR}/deps/jansson-build")
add_subdirectory("./src")
add_subdirectory("./test")
add_subdirectory("./benchmark")
add_subdirectory("./example")
include_directories("./include")
