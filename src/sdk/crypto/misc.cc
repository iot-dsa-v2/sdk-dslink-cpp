#include "dsa_common.h"

#include "misc.h"

#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

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
std::string base64_encode(uint8_t const* bytes_to_encode, unsigned int in_len) {
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

  std::string ret(bufferPtr->data, bufferPtr->length);

  BIO_free_all(b64);

  return ret;
}

std::string base64_decode(std::string const& encoded_string) {
  size_t in_len = encoded_string.size();
  BIO *bio, *b64;

  auto *buffer = new uint8_t[in_len];

  b64 = BIO_new(BIO_f_base64());
  bio = BIO_new_mem_buf(encoded_string.c_str(), in_len);
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_push(b64, bio);

  BIO_read(b64, buffer, in_len);
  BIO_flush(b64);

  std::string ret = std::string((const char*)buffer, in_len);

  delete [] buffer;

  BIO_free_all(b64);

  return ret;
}

std::string base64url(std::string str) {
  std::string out;
  size_t length = str.size();

  out.resize(length);
  for(size_t src_idx=0, dest_idx=0; src_idx<length; ++src_idx, ++dest_idx) {
    switch(str[src_idx]) {
    case '+':
      out[dest_idx] = '-';
      break;
    case '/':
      out[dest_idx] = '_';
      break;
    case '=':
      out.resize(dest_idx);
      return out;
    default:
      out[dest_idx] = str[src_idx];
    }
  }

  return out;
}

BufferPtr hex2bin(const char* src) {
  auto out = make_intrusive_<Buffer>();

  int i = 0;
  while (src[i] && src[i + 1]) {
    out->safe_append(char2int(src[i]) * 16 + char2int(src[i + 1]));
    i += 2;
  }

  return std::move(out);
}

BufferPtr gen_salt(int len) {
  uint8_t* out = new uint8_t[len];
  if (!RAND_bytes(out, len))
    throw std::runtime_error("Unable to generate salt");
  return std::move(make_intrusive_<Buffer>(out, len, len));
}
}  // namespace dsa
