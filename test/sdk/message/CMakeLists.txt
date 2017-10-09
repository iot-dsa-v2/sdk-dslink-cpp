# TESTS
set(TARGET_NAME message-unittests)
add_library(${TARGET_NAME} OBJECT
        message_value-test.cc
        handshake_f0_message-test.cc
        handshake_f1_message-test.cc
        handshake_f2_message-test.cc
        handshake_f3_message-test.cc
        handshake_message-test.cc
        subscribe_message-test.cc
        list_message-test.cc
        invoke_message-test.cc
        set_message-test.cc
        enum-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

include_directories(${CMAKE_SOURCE_DIR}/deps/msgpack/include)

