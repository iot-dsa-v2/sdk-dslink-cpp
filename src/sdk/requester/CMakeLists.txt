include_directories("..")
include_directories("../../../include")

add_library(dsa_requester OBJECT
 subscription_stream.cc
 )

set(EXTRA_LIBS ${EXTRA_LIBS} dsa_responder)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_responder> PARENT SCOPE)