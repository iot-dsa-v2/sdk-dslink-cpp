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

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type -Wno-deprecated")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror=return-type -Wno-deprecated")

    if (NOT CYGWIN)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fstack-protector-all")
    endif()
endif()

#set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SDK_PROJECT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SDK_PROJECT_INCLUDE_DIR ${SDK_PROJECT_SOURCE_DIR}/include)
set(SDK_PROJECT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

option(INCLUDE_BROKER "build dsa broker" ON)

option(INCLUDE_MODULE_PACK "Include set additional modules" OFF)

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/msgpack-build")
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/deps/jansson-build")
add_subdirectory("./src")
add_subdirectory("./test")
add_subdirectory("./benchmark")
add_subdirectory("./example")
include_directories("./include")
