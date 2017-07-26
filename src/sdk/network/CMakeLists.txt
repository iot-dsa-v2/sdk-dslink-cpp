include_directories("..")
include_directories("../../../include")

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
