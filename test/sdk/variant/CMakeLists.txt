# TESTS
set(TARGET_NAME variant-test)
add_library(${TARGET_NAME} OBJECT
        variant.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
