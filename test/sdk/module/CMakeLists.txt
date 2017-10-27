# TESTS
set(TARGET_NAME module-unittests)
add_library(${TARGET_NAME} OBJECT
        storage-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)


