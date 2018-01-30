#ifndef DSA_SDK_OPENSSL_H
#define DSA_SDK_OPENSSL_H


#if OPENSSL_VERSION_NUMBER < 0x10100000

#define EVP_MD_CTX_new new EVP_MD_CTX
#define EVP_MD_CTX_reset EVP_MD_CTX_init
#define EVP_MD_CTX_free(X) EVP_MD_CTX_cleanup(X); delete X;

#define HMAC_CTX_new new HMAC_CTX
#define HMAC_CTX_reset HMAC_CTX_init
#define HMAC_CTX_free(X) HMAC_CTX_cleanup(X); delete X;

#endif

#endif //DSA_SDK_OPENSSL_H
