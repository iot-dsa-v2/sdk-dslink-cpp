# TESTS
set(TARGET_NAME sdk_util)
add_library(${TARGET_NAME} OBJECT
        certificate-test.cc
        date_time.cc
        enable_shared-test.cc
        enable_intrusive-test.cc
        little_endian_test.cc
        path-test.cc
        buffer-test.cc
        shared_strand_ref-test.cc
        string-test.cc
        temp_file-test.cc
        )
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)
