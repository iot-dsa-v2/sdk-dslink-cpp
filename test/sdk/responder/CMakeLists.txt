# TESTS
set(TARGET_NAME responder-test)
add_library(${TARGET_NAME} OBJECT
        responder-subscribe.cc
        responder-qos-test.cc
        responder-list.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

