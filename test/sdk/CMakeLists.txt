#
set(SOURCES "")

#
foreach(dir "./crypto"
	    "./message"		
	    "./requester"		
	    "./responder"		
	    "./util"
        "./variant"
		"./network")
  add_subdirectory(${dir})
endforeach()

#
set(SOURCES ${SOURCES} PARENT_SCOPE)

set(TARGET_NAME "sdk-test")
add_library(${TARGET_NAME} ${SOURCES})
