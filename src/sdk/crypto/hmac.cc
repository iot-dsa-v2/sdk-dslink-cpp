#include <iostream>
#include "dsa_common.h"

#include "hmac.h"

#include "util/openssl.h"

namespace dsa {

static int i = 0;
void HMAC::init(const std::vector<uint8_t> &content) throw(
    const std::runtime_error &) {
  ctx = HMAC_CTX_create();
  const EVP_MD *md = EVP_sha256();

  HMAC_CTX_init(ctx);

  if (!HMAC_Init_ex(ctx, reinterpret_cast<const uint8_t *>(content.data()),
                    (int)content.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");
}

HMAC::HMAC(const std::vector<uint8_t> &data) throw(const std::runtime_error &) {
  init(data);
  initialized = true;
}

HMAC::~HMAC() {
  HMAC_CTX_cleanup(ctx);
#if OPENSSL_VERSION_NUMBER < 0x10100000
  delete ctx;
#endif
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