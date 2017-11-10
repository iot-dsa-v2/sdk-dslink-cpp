# TESTS
set(TARGET_NAME network-unittests)
add_library(${TARGET_NAME} OBJECT
        tcp_server-test.cc
        reconnect.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)