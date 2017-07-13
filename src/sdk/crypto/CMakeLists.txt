include_directories(${PROJECT_INCLUDE_DIR}/dsa/crypto)

if(UNIX)
  set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})
else()
  find_package(OpenSSL REQUIRED)
endif()

add_library(dsa_crypto OBJECT
  ecdh.cc
  hash.cc
  hmac.cc 
  misc.cc)

set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENSSL_LIBRARIES} dsa_crypto)
set(SOURCES ${SOURCES} $<TARGET_OBJECTS:dsa_crypto> PARENT_SCOPE)
