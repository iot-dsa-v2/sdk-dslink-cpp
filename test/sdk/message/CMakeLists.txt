# TESTS
set(TARGET_NAME message-unittests)
add_library(${TARGET_NAME} OBJECT
        subscribe_message-test.cc)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)