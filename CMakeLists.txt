cmake_minimum_required(VERSION 3.7)


if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.1)
    set(CMAKE_CXX_STANDARD 14)
else ()
    set(CMAKE_CXX_STANDARD 11)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDSA_DEBUG")

set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

add_subdirectory("${CMAKE_SOURCE_DIR}/deps/msgpack-build")

add_subdirectory("./src")
add_subdirectory("./test")
#add_subdirectory("./example")
include_directories("./include")
