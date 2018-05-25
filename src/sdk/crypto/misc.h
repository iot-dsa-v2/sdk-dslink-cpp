#ifndef DSA_SDK_CRYPTO_MISC_H
#define DSA_SDK_CRYPTO_MISC_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>
#include <vector>

#include "util/buffer.h"

namespace dsa {

std::vector<uint8_t> base64_decode(const string_ &encoded_string);

string_ base64_encode(const uint8_t *bytes_to_encode, unsigned int in_len,
                      bool url = true);
inline string_ base64_encode(const std::vector<uint8_t> &data,
                             bool url = true) {
  return base64_encode(data.data(), data.size(), url);
}
void gen_salt(uint8_t *data, size_t len);
string_ hex2bin(const char *src);

bool validate_token_auth(const string_ &id, const string_ &token,
                         const string_ &auth);
string_ generate_auth_token(const string_ &id, const string_ &token);
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_MISC_H
