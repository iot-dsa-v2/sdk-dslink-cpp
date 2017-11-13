# TESTS
set(TARGET_NAME web-server-unittests)
add_library(${TARGET_NAME} OBJECT
            web_server_test.cc
		)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)