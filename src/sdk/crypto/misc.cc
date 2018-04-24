#include "dsa_common.h"

#include "misc.h"

#include <sstream>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include "hash.h"

static inline bool is_base64(uint8_t c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static int char2int(char input) {
  if (input >= '0' && input <= '9') return input - '0';
  if (input >= 'A' && input <= 'F') return input - 'A' + 10;
  if (input >= 'a' && input <= 'f') return input - 'a' + 10;
  throw std::invalid_argument("Invalid input string");
}

namespace dsa {
string_ base64_encode(uint8_t const *bytes_to_encode, unsigned int in_len) {
  BIO *bio, *b64;
  BUF_MEM *bufferPtr;

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new(BIO_s_mem());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bio);

  BIO_write(b64, bytes_to_encode, in_len);
  BIO_flush(b64);
  BIO_get_mem_ptr(b64, &bufferPtr);
  BIO_set_close(b64, BIO_NOCLOSE);

  string_ ret(bufferPtr->data, bufferPtr->length);

  BIO_free_all(b64);
  BUF_MEM_free(bufferPtr);

  return ret;
}

string_ base64_decode(string_ const &encoded_string) {
  size_t in_len = encoded_string.size();
  BIO *bio, *b64;

  auto *buffer = new uint8_t[in_len];

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_mem_buf(encoded_string.c_str(), in_len);
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bio);

  size_t out_len = BIO_read(b64, buffer, in_len);
  BIO_flush(b64);

  string_ ret = string_((const char *)buffer, out_len);

  delete[] buffer;

  BIO_free_all(b64);

  return ret;
}

void base64_url_convert_(string_ &str) {
  size_t length = str.size();
  for (size_t idx = 0; idx < length; ++idx) {
    switch (str[idx]) {
      case '+':
        str[idx] = '-';
        break;
      case '/':
        str[idx] = '_';
        break;
      case '=':
        str.resize(idx);
        return;
      default:;
    }
  }
}

string_ base64_url_convert(const string_ &str) {
  string_ out(str);
  base64_url_convert_(out);
  return std::move(out);
}

string_ hex2bin(const char *src) {
  std::stringstream out;

  int i = 0;
  while (src[i] && src[i + 1]) {
    out << (uint8_t)(char2int(src[i]) * 16 + char2int(src[i + 1]));
    i += 2;
  }

  return out.str();
}

void gen_salt(uint8_t *data, size_t len) { RAND_bytes(data, len); }

bool validate_token_auth(const string_ &id, const string_ &token,
                         const string_ &auth) {
  Hash hash;
  std::vector<uint8_t> v;
  v.insert(v.end(), id.begin(), id.end());
  v.insert(v.end(), token.begin(), token.end());
  hash.update(v);
  auto hash_result = hash.digest_base64();
  return hash_result == auth;
}
string_ generate_auth_token(const string_ &id, const string_ &token) {
  Hash hash;
  std::vector<uint8_t> v;
  v.insert(v.end(), id.begin(), id.end());
  v.insert(v.end(), token.begin(), token.end());
  hash.update(v);
  auto hash_result = hash.digest_base64();
  return token.substr(0, 16) + hash_result;
}
}  // namespace dsa
