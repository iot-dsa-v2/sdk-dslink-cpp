#include "dsa_common.h"

#include "hash.h"

#include <regex>

#include "misc.h"

namespace dsa {
Hash::Hash(const char *hash_type) throw(const std::runtime_error &) : finalized(false) {
  mdctx = new EVP_MD_CTX;
  const EVP_MD *md = EVP_get_digestbyname(hash_type);
  if (md == nullptr) {
    delete mdctx;
    throw std::runtime_error("invalid hash type");
  }
  EVP_MD_CTX_init(mdctx);
  if (EVP_DigestInit_ex(mdctx, md, nullptr) <= 0) {
    delete mdctx;
    throw std::runtime_error("something went wrong initializing digest");
  }
}

Hash::~Hash() {
  delete mdctx;
}

void Hash::update(const Buffer& content) {
  EVP_DigestUpdate(mdctx, content.data(), content.size());
}

std::string Hash::digest_base64() throw(const std::runtime_error &) {
  if (finalized) throw std::runtime_error("digest already called");

  uint8_t md_value[EVP_MAX_MD_SIZE];
  uint32_t md_len;
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  finalized = true;

  EVP_MD_CTX_cleanup(mdctx);

  std::string out = base64_encode(md_value, md_len);
  return out;
}
}  // namespace dsa
