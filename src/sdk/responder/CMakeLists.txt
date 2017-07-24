include_directories("..")

add_library(dsa_responder OBJECT
        responder.cc
        node_model_manager.cc
        node_model.cc
        node_state_manager.cc
        node_state.cc
        subscription_queue.cc)

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_responder> PARENT_SCOPE)
