# TESTS
set(TARGET_NAME sdk_util)
add_library(${TARGET_NAME} OBJECT
        enable_shared-test.cc)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
