include_directories("..")
include_directories("../../../include")

add_library(dsa_requester OBJECT
        incoming_message_stream.cc
 )

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_responder> PARENT_SCOPE)
