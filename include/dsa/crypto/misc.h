#ifndef DSA_SDK_CRYPTO_MISC_H_
#define DSA_SDK_CRYPTO_MISC_H_

#include <string>
#include <vector>

#include "util.h"

namespace dsa {
std::string base64url(std::string str);
std::string base64_decode(std::string const &encoded_string);
std::string base64_encode(unsigned char const *bytes_to_encode,
                          unsigned int in_len);
BufferPtr gen_salt(int len);
BufferPtr hex2bin(const char *src);
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_MISC_H_