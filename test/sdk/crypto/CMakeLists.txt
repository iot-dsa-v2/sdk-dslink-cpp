# TESTS
set(TARGET_NAME crypto-unittests)
add_library(${TARGET_NAME} OBJECT
#        ecdh-test.cc
#		handshake-test.cc
#		hash-test.cc)
		)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:${TARGET_NAME}> PARENT_SCOPE)