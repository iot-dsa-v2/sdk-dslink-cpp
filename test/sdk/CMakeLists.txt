#
set(SOURCES "")

#
foreach(dir
		"./crypto"
		"./message"
	    "./module"
		"./util"
		"./variant"
		"./network"
	    "./responder"
#            "./web_server"
       )
  add_subdirectory(${dir})
endforeach()

#
add_library(dsa_test_util OBJECT
		async_test.cc
		test_config.cc
		)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_test_util>)

#
set(SOURCES ${SOURCES} PARENT_SCOPE)

set(TARGET_NAME "sdk-test")
add_library(${TARGET_NAME} ${SOURCES})
