include_directories(${LIB_CRYPTO})

list(APPEND CryptoSourceFiles
        ecdh.cc
        hash.cc
        hmac.cc
        misc.cc)

foreach(file ${CryptoSourceFiles})
  list(APPEND ProjectSourceFiles ${LIB_CRYPTO}/${file})
endforeach()

# DEPENDENCIES
## OpenSSL v1.0.2l
if(UNIX)
  set(OPENSSL_ROOT_DIR "/usr/local/Cellar/openssl/1.0.2l")
endif()
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
