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
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES} PARENT_SCOPE)

add_library(dsa_network OBJECT
        app.cc
        gracefully_closable.cc
        connection.cc
        session.cc
        server.cc
        security_context.cc
        session_manager.cc
        tcp_connection.cc
        tcp_server.cc)

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_network> PARENT_SCOPE)
