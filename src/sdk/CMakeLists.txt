cmake_minimum_required(VERSION 2.7)

set(CMAKE_CXX_STANDARD 11)

# Add external cmake modules
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake")

# DEPENDENCIES
## OpenSSL v1.0.2l
if (UNIX)
#    set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
    set(OPENSSL_ROOT_DIR "/usr/local/ssl")
endif ()
find_package(OpenSSL REQUIRED)
set(OPENSSL_INCLUDE_DIR "/usr/local/ssl/include")
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

## Boost Library v1.64
find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
#    set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
#    set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
    set(BOOST_INCLUDEDIR "/usr/local/include")
    set(BOOST_LIBRARYDIR "/usr/local/lib")
endif ()
find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time regex)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

## MsgPack
if (UNIX)
    set(MSGPACK_INCLUDE_DIR "/usr/local/include")
    find_package(Msgpack REQUIRED)
    include_directories(SYSTEM ${MSGPACK_INCLUDE_DIRS})
endif ()
set(EXTRA_LIBS ${EXTRA_LIBS} ${MSGPACK_LIBRARY})

# Standard DSA v2 headers
include_directories(".")
include_directories(${PROJECT_INCLUDE_DIR})

# LIBRARIES:
set(LIB_UTIL "./util")
set(LIB_VARIANT "./variant")
set(LIB_CRYPTO "./crypto")
set(LIB_MESSAGE "./message")
set(LIB_NETWORK "./network")
set(LIB_REQUESTER "./requester")
set(LIB_RESPONDER "./responder")
set(LIB_CORE "./core")

# SOURCE FILES
set(ProjectSourceFiles)

# EXTERNAL LIBRARIES
set(ExtraLibs)

include(${LIB_UTIL}/CMakeListsInclude.txt)
include(${LIB_VARIANT}/CMakeListsInclude.txt)
include(${LIB_CRYPTO}/CMakeListsInclude.txt)
include(${LIB_MESSAGE}/CMakeListsInclude.txt)
include(${LIB_NETWORK}/CMakeListsInclude.txt)
include(${LIB_REQUESTER}/CMakeListsInclude.txt)
include(${LIB_RESPONDER}/CMakeListsInclude.txt)
include(${LIB_CORE}/CMakeListsInclude.txt)

add_library(dsa_sdk STATIC ${ProjectSourceFiles})
target_link_libraries(dsa_sdk ${EXTRA_LIBS})
#TODO target_compile_features(dsa_sdk PRIVATE cxx_range_for)
install(TARGETS dsa_sdk
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
