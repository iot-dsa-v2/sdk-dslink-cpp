include_directories(${PROJECT_INCLUDE_DIR}/dsa/util)

add_library(dsa_util OBJECT
  buffer.cc)

message("${PROJECT_INCLUDE_DIR}/dsa/util")

set(EXTRA_LIBS ${EXTRA_LIBS} dsa_util)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_util> PARENT_SCOPE)