cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 11)

# Standard DSA v2 headers
include_directories(".")
include_directories("../../include")

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
option(USE_REQUESTER "Include dsa requester library" ON)
option(USE_RESPONDER "Include dsa responder library" ON)

# SOURCE FILES
set(ProjectSourceFiles "")

# EXTERNAL LIBRARIES
set(EXTRA_LIBS)

# CRYPTO LIBRARY:
if(USE_CRYPTO)
  message(">> adding crypto library...")
  include(${LIB_CRYPTO}/CMakeLists.txt)
endif()

# MESSAGE LIBRARY:
if(USE_MESSAGE)
  message(">> adding message library...")
  include(${LIB_MESSAGE}/CMakeLists.txt)
endif()

# NETWORK LIBRARY:
if(USE_NETWORK)
  message(">> adding network library...")
  if($<NOT:USE_CRYPTO>)
    message(">> adding crypto library as network library dependency...")
    include(${LIB_CRYPTO}/CMakeLists.txt)
  endif()
  include(${LIB_NETWORK}/CMakeLists.txt)
endif()

# REQUESTER LIBRARY:
if(USE_REQUESTER)
  message(">> adding requester library...")
  if($<NOT:USE_NETWORK>)
    message(">> adding network library as requester library dependency...")
    include(${LIB_NETWORK}/CMakeLists.txt)
  endif()
  if($<NOT:USE_MESSAGE>)
    message(">> adding message library as requester library dependency...")
    include(${LIB_MESSAGE}/CMakeLists.txt)
  endif()
  include(${LIB_REQUESTER}/CMakeLists.txt)
endif()

# RESPONDER LIBRARY:
if(USE_RESPONDER)
  message(">> adding responder library...")
  if($<NOT:USE_REQUESTER>)
    if($<NOT:USE_NETWORK>)
      message(">> adding network library as responder library dependency...")
      include(${LIB_NETWORK}/CMakeLists.txt)
    endif()
    if($<NOT:USE_MESSAGE>)
      message(">> adding network library as responder library dependency...")
      include(${LIB_MESSAGE}/CMakeLists.txt)
    endif()
  endif()
  include(${LIB_RESPONDER}/CMakeLists.txt)
endif()

# UTIL LIBRARY
message(">> adding util library...")
include(${LIB_UTIL}/CMakeLists.txt)

add_library(dsa_sdk STATIC ${ProjectSourceFiles})
target_link_libraries(dsa_sdk ${EXTRA_LIBS})
target_compile_features(dsa_sdk PRIVATE cxx_range_for)
install (TARGETS dsa_sdk
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
