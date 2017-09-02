# TESTS
set(TARGET_NAME requester-test)
add_library(${TARGET_NAME} OBJECT
        requester-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

