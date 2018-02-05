#ifndef DSA_SDK_CRYPTO_MISC_H
#define DSA_SDK_CRYPTO_MISC_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <vector>

#include "util/buffer.h"

namespace dsa {
string_ base64_url_convert(const string_ &str);
string_ base64_decode(const string_ &encoded_string);
string_ base64_encode(const uint8_t *bytes_to_encode, unsigned int in_len);
void gen_salt(uint8_t *data, size_t len);
string_ hex2bin(const char *src);
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_MISC_H
