include_directories("..")
include_directories("../../../include")

find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT} PARENT_SCOPE)
if (UNIX)
    set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
    set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
endif ()
find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS} PARENT_SCOPE)

if(UNIX)
    set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
endif()

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENSSL_LIBRARIES} PARENT_SCOPE)

add_library(dsa_network OBJECT
        app.cc
        connection.cc
        session.cc
        server.cc
        security_context.cc
        tcp_connection.cc
        tcp_server.cc)

set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES} dsa_network)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_network> PARENT_SCOPE)
