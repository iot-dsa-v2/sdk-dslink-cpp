include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

## Boost Library v1.66
find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
    set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
    set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
endif ()
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# Standard DSA v2 headers
include_directories(".")

# LIBRARIES:
set(LIB_UTIL "./util")
set(LIB_VARIANT "./variant")
set(LIB_CRYPTO "./crypto")
set(LIB_MESSAGE "./message")
set(LIB_STREAM "./stream")
set(LIB_NETWORK "./network")
set(LIB_REQUESTER "./requester")
set(LIB_RESPONDER "./responder")
set(LIB_CORE "./core")
set(LIB_MODULE "./module")
set(LIB_DSLINK "./dslink")
set(LIB_WEBSERVER "./web_server")

# SOURCE FILES
set(ProjectSourceFiles dsa_common.cc dsa_common.h)

# EXTERNAL LIBRARIES
set(ExtraLibs)

include(${LIB_UTIL}/CMakeListsInclude.txt)
include(${LIB_VARIANT}/CMakeListsInclude.txt)
include(${LIB_CRYPTO}/CMakeListsInclude.txt)
include(${LIB_MESSAGE}/CMakeListsInclude.txt)
include(${LIB_STREAM}/CMakeListsInclude.txt)
include(${LIB_NETWORK}/CMakeListsInclude.txt)
include(${LIB_REQUESTER}/CMakeListsInclude.txt)
include(${LIB_RESPONDER}/CMakeListsInclude.txt)
include(${LIB_CORE}/CMakeListsInclude.txt)
include(${LIB_MODULE}/CMakeListsInclude.txt)
include(${LIB_DSLINK}/CMakeListsInclude.txt)
include(${LIB_WEBSERVER}/CMakeListsInclude.txt)

add_library(dsa_sdk STATIC ${ProjectSourceFiles}
        $<TARGET_OBJECTS:msgpackc-object>
        $<TARGET_OBJECTS:jansson-object>)
target_link_libraries(dsa_sdk ${EXTRA_LIBS})
target_compile_features(dsa_sdk PRIVATE cxx_range_for)
install(TARGETS dsa_sdk
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
