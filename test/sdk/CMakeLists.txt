#
set(SOURCES "")

#
foreach(dir "./crypto"
	    "./message"		
	    "./util"
        "./variant"
#		"./network"
#		"./requester"
#	    "./responder"
		)
  add_subdirectory(${dir})
endforeach()

#
set(SOURCES ${SOURCES} PARENT_SCOPE)

set(TARGET_NAME "sdk-test")
add_library(${TARGET_NAME} ${SOURCES})
