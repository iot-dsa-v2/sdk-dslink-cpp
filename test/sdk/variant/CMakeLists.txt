# TESTS
set(TARGET_NAME variant-test)
add_library(${TARGET_NAME} OBJECT
        variant.cc
        msgpack_encode.cc
        msgpack_decode.cc
#        json_encode.cc
#        json_decode.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)

include_directories(${CMAKE_SOURCE_DIR}/deps/msgpack/include)

