include_directories("..")

add_library(dsa_message OBJECT
        base_message.cc
        message_factory.cc
        static_header.cc
        dynamic_header.cc
        request/invoke_request_message.cc
        request/subscribe_request_message.cc
        request/list_request_message.cc
        request/set_request_message.cc
        response/invoke_response_message.cc
        response/subscribe_response_message.cc
        response/list_response_message.cc
        response/set_response_message.cc
)

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_message> PARENT_SCOPE)