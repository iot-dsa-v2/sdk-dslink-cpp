#include <iostream>
#include "dsa_common.h"

#include "hmac.h"

#include "util/openssl.h"

namespace dsa {

HMAC::HMAC(const std::vector<uint8_t> &data) throw(const std::runtime_error &) {
  ctx = HMAC_CTX_new();
  const EVP_MD *md = EVP_sha256();

  HMAC_CTX_reset(ctx);

  if (!HMAC_Init_ex(ctx, reinterpret_cast<const uint8_t *>(data.data()),
                    (int)data.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");

  initialized = true;
}


HMAC::~HMAC() {
  //TODO: for openssl 1.1 there is illogical memleak on it, maybe bug in openssl
  HMAC_CTX_free(ctx);
}

void HMAC::update(const std::vector<uint8_t> &content) throw(
    const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");
  int r = HMAC_Update(ctx, reinterpret_cast<const uint8_t *>(content.data()),
                      content.size());
  if (r == 0) throw std::runtime_error("Failed to update HMAC");
}

std::vector<uint8_t> HMAC::digest() throw(const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");

  std::vector<uint8_t> out;
  out.resize(EVP_MAX_MD_SIZE);
  unsigned int size = 0;

  int r = HMAC_Final(ctx, reinterpret_cast<uint8_t *>(&out[0]), &size);
  if (r == 0) {
    throw std::runtime_error("Failed to get digest");
  }
  initialized = false;

  out.resize(size);
  return std::move(out);
}
}  // namespace dsa