cmake_minimum_required(VERSION 3.7)
# project(dsa_sdk)

set(CMAKE_CXX_STANDARD 11)

# LIBRARIES:
set(LIB_CRYPTO "./crypto")
set(LIB_MESSAGE "./message")
set(LIB_NETWORK "./network")
set(LIB_REQUESTER "./requester")
set(LIB_RESPONDER "./responder")
set(LIB_UTIL "./util")

# LIBRARY OPTIONS:
option(USE_CRYPTO "Include dsa crypto library" ON)
option(USE_MESSAGE "Include dsa message library" ON)
option(USE_NETWORK "Include dsa network library" ON)
option(USE_REQUESTER "Include dsa requester library" OFF)
option(USE_RESPONDER "Include dsa responder library" ON)

set(SOURCES "")
set(EXTRA_LIBS "" CACHE INTERNAL "")

# CRYPTO LIBRARY:
if(USE_CRYPTO)
  message(">> adding crypto library...")
  add_subdirectory(${LIB_CRYPTO})
endif()

# MESSAGE LIBRARY:
if(USE_MESSAGE)
  message(">> adding message library...")
  add_subdirectory(${LIB_MESSAGE})
endif()

# NETWORK LIBRARY:
if(USE_NETWORK)
  message(">> adding network library...")
  if($<NOT:USE_CRYPTO>)
    message(">> adding crypto library as network library dependency...")
    add_subdirectory(${LIB_CRYPTO})
  endif()
  add_subdirectory(${LIB_NETWORK})
endif()

# REQUESTER LIBRARY:
if(USE_REQUESTER)
  message(">> adding requester library...")
  if($<NOT:USE_NETWORK>)
    message(">> adding network library as requester library dependency...")
    add_subdirectory(${LIB_NETWORK})
  endif()
  if($<NOT:USE_MESSAGE>)
    message(">> adding message library as requester library dependency...")
    add_subdirectory(${LIB_MESSAGE})
  endif()
  add_subdirectory(${LIB_REQUESTER})
endif()

# RESPONDER LIBRARY:
if(USE_RESPONDER)
  message(">> adding responder library...")
  if($<NOT:USE_REQUESTER>)
    if($<NOT:USE_NETWORK>)
      message(">> adding network library as responder library dependency...")
      add_subdirectory(${LIB_NETWORK})
    endif()
    if($<NOT:USE_MESSAGE>)
      message(">> adding network library as responder library dependency...")
      add_subdirectory(${LIB_MESSAGE})
    endif()
  endif()
  add_subdirectory(${LIB_RESPONDER})
endif()

# UTIL LIBRARY
message(">> adding util library...")
add_subdirectory(${LIB_UTIL})

add_library(dsa_sdk ${SOURCES})
target_link_libraries(dsa_sdk ${EXTRA_LIBS})
target_compile_features(dsa_sdk PRIVATE cxx_range_for)
install (TARGETS dsa_sdk
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
