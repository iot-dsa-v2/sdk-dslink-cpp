include_directories(${PROJECT_INCLUDE_DIR}/dsa/message)

add_library(dsa_message OBJECT
  abstract_message.cc
  invoke_message.cc 
  list_message.cc 
  list_message.cc 
  message_factory.cc 
  observe_message.cc 
  set_message.cc 
  subscribe_message.cc)

set(EXTRA_LIBS ${EXTRA_LIBS} dsa_message)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_message> PARENT_SCOPE)