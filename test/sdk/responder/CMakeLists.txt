# TESTS
set(TARGET_NAME responder-test)
add_library(${TARGET_NAME} OBJECT
        subscribe-test.cc
        qos-test.cc
        responder-list-test.cc
        node_model-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

