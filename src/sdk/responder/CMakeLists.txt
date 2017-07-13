include_directories(${PROJECT_INCLUDE_DIR}/dsa/responder)

add_library(dsa_responder OBJECT
  node_model_manager.cc 
  node_model.cc 
  node_state_manager.cc 
  node_state.cc 
  subscription_queue.cc)

set(EXTRA_LIBS ${EXTRA_LIBS} dsa_responder)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_responder> PARENT SCOPE)