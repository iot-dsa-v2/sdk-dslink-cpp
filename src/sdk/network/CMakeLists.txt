include_directories("${LIB_NETWORK}")

list(APPEND NetworkSourceFiles
        app.cc
        message_stream.cc
        gracefully_closable.cc
        connection.cc
        session.cc
        server.cc
        security_context.cc
        session_manager.cc
        tcp_connection.cc
        tcp_server.cc)

foreach(file ${NetworkSourceFiles})
    list(APPEND ProjectSourceFiles ${LIB_NETWORK}/${file})
endforeach()

# DEPENDENCIES
## Boost Library v1.64
find_package(Threads)
set(EXTRA_LIBS ${EXTRA_LIBS} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
    set(BOOST_INCLUDEDIR "/usr/local/Cellar/boost/1.64.0_1/include")
    set(BOOST_LIBRARYDIR "/usr/local/Cellar/boost/1.64.0_1/lib")
endif ()
find_package(Boost 1.6 REQUIRED COMPONENTS system thread date_time)
include_directories(${Boost_INCLUDE_DIRS})