#include "dsa_common.h"

#include "hash.h"

#include <regex>
#include <iostream>
#include "misc.h"

#include "util/openssl.h"

namespace dsa {


static int a = 0;

Hash::Hash() throw(const std::runtime_error &) : finalized(false) {
  mdctx = EVP_MD_CTX_create();
  const EVP_MD *md = EVP_sha256();
  EVP_MD_CTX_init(mdctx);
  if (EVP_DigestInit_ex(mdctx, md, nullptr) <= 0) {
    EVP_MD_CTX_cleanup(mdctx);
    throw std::runtime_error("something went wrong initializing digest");
  }
}

Hash::~Hash() {
  EVP_MD_CTX_cleanup(mdctx);
#if OPENSSL_VERSION_NUMBER < 0x10100000
  delete mdctx;
#endif
}

void Hash::update(const std::vector<uint8_t> &content) {
  EVP_DigestUpdate(mdctx, (uint8_t *)&content[0], content.size());
}

string_ Hash::digest_base64() throw(const std::runtime_error &) {
  if (finalized) throw std::runtime_error("digest already called");

  uint8_t md_value[EVP_MAX_MD_SIZE];
  uint32_t md_len;
  EVP_DigestFinal_ex(mdctx, md_value, &md_len);
  finalized = true;

  string_ out = base64_encode(md_value, md_len);
  return out;
}
}  // namespace dsa
