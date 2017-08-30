#ifndef DSA_SDK_CRYPTO_MISC_H_
#define DSA_SDK_CRYPTO_MISC_H_

#include <string>
#include <vector>

#include "util/buffer.h"

namespace dsa {
std::string base64_url_convert(const std::string &str);
std::string base64_decode(const std::string &encoded_string);
std::string base64_encode(const uint8_t *bytes_to_encode, unsigned int in_len);
void gen_salt(uint8_t *data, size_t len);
std::string hex2bin(const char *src);
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_MISC_H_
