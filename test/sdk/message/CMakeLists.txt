# TESTS
set(TARGET_NAME message-unittests)
add_library(${TARGET_NAME} OBJECT
        message_value-test.cc
        subscribe_message-test.cc
        set_message-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)