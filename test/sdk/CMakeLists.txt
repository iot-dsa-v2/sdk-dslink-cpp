#
set(SOURCES "")

#
foreach(dir "./crypto"
	    "./message"		
	    "./requester"		
	    "./responder"		
	    "./util"
#[[		"./variant"
	this is commented out because varient is using std::make_unique which is a C++14 feature
	and therefore is causing a compiler error]]
		"./network")
  add_subdirectory(${dir})
endforeach()

#
set(SOURCES ${SOURCES} PARENT_SCOPE)

set(TARGET_NAME "sdk-test")
add_library(${TARGET_NAME} ${SOURCES})
