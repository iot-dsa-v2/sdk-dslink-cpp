#include "crypto.h"

#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <openssl/evp.h>

namespace dsa {
hash::hash(const char *hash_type) : finalized(false) {
  const EVP_MD *md = EVP_get_digestbyname(hash_type);
  if (md == nullptr) throw std::runtime_error("invalid hash type");
  // mdctx = EVP_MD_CTX_create();
  EVP_MD_CTX_init(&mdctx);
  if (EVP_DigestInit_ex(&mdctx, md, nullptr) <= 0)
    throw std::runtime_error("something went wrong initializing digest");
}

hash::~hash() {
  // EVP_MD_CTX_destroy(mdctx);
}

void hash::update(std::vector<byte> data) {
  EVP_DigestUpdate(&mdctx, &data[0], data.size());
}

std::string hash::digest_base64() {
  if (finalized) throw std::runtime_error("digest already called");

  byte *md_value = new byte[EVP_MAX_MD_SIZE];
  uint md_len;
  EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
  finalized = true;

  EVP_MD_CTX_cleanup(&mdctx);

  std::string out = base64_encode(md_value, md_len);
  delete[] md_value;
  return out;
}
}  // namespace dsa
