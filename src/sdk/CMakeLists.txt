cmake_minimum_required(VERSION 3.7)
# project(dsa_sdk)

set(CMAKE_CXX_STANDARD 11)

include_directories(".")
include_directories("../../include")

find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
    set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
    set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
endif ()
find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS})


if(UNIX)
  set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
endif()

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})


set(SOURCES "")
set(EXTRA_LIBS "" CACHE INTERNAL "")

include("./util/CMakeListsInclude.txt")
include("./crypto/CMakeListsInclude.txt")
include("./message/CMakeListsInclude.txt")
include("./network/CMakeListsInclude.txt")
include("./requester/CMakeListsInclude.txt")
include("./responder/CMakeListsInclude.txt")


add_library(dsa_sdk STATIC ${SOURCES})
target_link_libraries(dsa_sdk ${EXTRA_LIBS})
target_compile_features(dsa_sdk PRIVATE cxx_range_for)
install (TARGETS dsa_sdk
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
