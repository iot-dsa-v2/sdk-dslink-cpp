include_directories("..")

add_library(dsa_util OBJECT
        buffer.cc
        )

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_util> PARENT_SCOPE)