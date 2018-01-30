#include "dsa_common.h"

#include "hash.h"

#include <regex>
#include <iostream>
#include "misc.h"

#include "util/openssl.h"

namespace dsa {

Hash::Hash() throw(const std::runtime_error &) : finalized(false) {
  mdctx = EVP_MD_CTX_new();
  const EVP_MD *md = EVP_sha256();

  EVP_MD_CTX_reset(mdctx);

  if (EVP_DigestInit_ex(mdctx, md, nullptr) <= 0) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("something went wrong initializing digest");
  }
}

Hash::~Hash() {
  EVP_MD_CTX_free(mdctx);
}

void Hash::update(const std::vector<uint8_t> &content) {
  if (finalized) throw std::runtime_error("Hash has been finalized already");
  int r = EVP_DigestUpdate(mdctx, (uint8_t *)&content[0], content.size());
  if (r == 0) throw std::runtime_error("Failed to update Hash");
}

string_ Hash::digest_base64() throw(const std::runtime_error &) {
  if (finalized) throw std::runtime_error("digest already called");

  uint8_t md_value[EVP_MAX_MD_SIZE];
  uint32_t md_len;
  int r = EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  if (r == 0) {
    throw std::runtime_error("Failed to get digest");
  }
  finalized = true;

  string_ out = base64_encode(md_value, md_len);
  return out;
}
}  // namespace dsa
