#ifndef DSA_SDK_OPENSSL_H
#define DSA_SDK_OPENSSL_H


#if OPENSSL_VERSION_NUMBER < 0x10100000

#define HMAC_CTX_create new HMAC_CTX

#else

#define EVP_MD_CTX_cleanup EVP_MD_CTX_free

#define HMAC_CTX_create HMAC_CTX_new
#define HMAC_CTX_init HMAC_CTX_reset
#define HMAC_CTX_cleanup HMAC_CTX_free

#endif

#endif //DSA_SDK_OPENSSL_H
