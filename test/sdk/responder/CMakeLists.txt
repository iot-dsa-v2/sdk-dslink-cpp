# TESTS
set(TARGET_NAME responder-test)
add_library(${TARGET_NAME} OBJECT
        subscribe-test.cc
        qos-test.cc
        invoke-test.cc
        list-test.cc
        set-test.cc
        node_model-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

