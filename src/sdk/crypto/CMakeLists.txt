include_directories("..")
include_directories("../../../include")

if(UNIX)
  set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
endif()

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENSSL_LIBRARIES} PARENT SCOPE)

add_library(dsa_crypto OBJECT
  ecdh.cc
  hash.cc
  hmac.cc 
  misc.cc)

set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_crypto> PARENT_SCOPE)
