#include "dsa_common.h"

#include "hmac.h"

namespace dsa {
void HMAC::init(const char* alg, const std::string &content) throw(const std::runtime_error &) {
  ctx = new HMAC_CTX;
  const EVP_MD* md = EVP_get_digestbyname(alg);
  if (md == nullptr) throw std::runtime_error("Failed to initialize HMAC");

  HMAC_CTX_init(ctx);

  if (!HMAC_Init_ex(ctx, reinterpret_cast<const uint8_t *>(content.data()), (int)content.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");
}

HMAC::HMAC(const char* alg, const std::string &data) throw(const std::runtime_error &) {
  init(alg, data);
  initialized = true;
}

HMAC::~HMAC() {
  delete ctx;
}

void HMAC::update(const std::string &content) throw(const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");
  int r = HMAC_Update(ctx, reinterpret_cast<const uint8_t *>(content.data()), content.size());
  if (r == 0) throw std::runtime_error("Failed to update HMAC");
}

std::string HMAC::digest() throw(const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");

  std::string out;
  out.resize(EVP_MAX_MD_SIZE);
  unsigned int size = 0;

  int r = HMAC_Final(ctx, reinterpret_cast<uint8_t*>(&out[0]), &size);
  if (r == 0) {
    throw std::runtime_error("Failed to get digest");
  }
  initialized = false;
  HMAC_CTX_cleanup(ctx);

  out.resize(size);
  return std::move(out);
}
}  // namespace dsa