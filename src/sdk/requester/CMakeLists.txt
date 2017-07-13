include_directories(${PROJECT_INCLUDE_DIR}/dsa/requester)

add_library(dsa_requester OBJECT
 #[[...source files]])

set(EXTRA_LIBS ${EXTRA_LIBS} dsa_responder)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_responder> PARENT SCOPE)