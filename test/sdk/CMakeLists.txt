#
set(SOURCES "")

#
foreach(dir
		"./crypto"
	    "./message"
	    "./util"
        "./variant"
		"./network"
		"./requester"
	    "./responder"
		)
  add_subdirectory(${dir})
endforeach()


add_library(dsa_async_test OBJECT
		async_test.cc
		)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_async_test>)

#
set(SOURCES ${SOURCES} PARENT_SCOPE)

set(TARGET_NAME "sdk-test")
add_library(${TARGET_NAME} ${SOURCES})
