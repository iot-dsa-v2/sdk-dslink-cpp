
include_directories("../src/sdk")
include_directories("../include")


add_executable(requester requester/requester.cc)
add_executable(responder responder/responder.cc)
