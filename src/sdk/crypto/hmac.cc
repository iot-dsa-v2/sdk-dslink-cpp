#include "dsa_common.h"

#include "hmac.h"

namespace dsa {
void HMAC::init(const char* alg, Buffer& content) throw(const std::runtime_error &) {
  ctx = new HMAC_CTX;
  const EVP_MD* md = EVP_get_digestbyname(alg);
  if (md == nullptr) throw std::runtime_error("Failed to initialize HMAC");

  HMAC_CTX_init(ctx);

  if (!HMAC_Init_ex(ctx, content.data(), (int)content.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");
}

HMAC::HMAC(const char* alg, Buffer& data) throw(const std::runtime_error &) {
  init(alg, data);
  initialized = true;
}

HMAC::~HMAC() {
  delete ctx;
}

void HMAC::update(const Buffer& content) throw(const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");
  int r = HMAC_Update(ctx, content.data(), content.size());
  if (r == 0) throw std::runtime_error("Failed to update HMAC");
}

BufferPtr HMAC::digest() throw(const std::runtime_error &) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");

  auto out = new uint8_t[EVP_MAX_MD_SIZE];
  unsigned int size = 0;

  int r = HMAC_Final(ctx, out, &size);
  if (r == 0) {
    delete[] out;
    throw std::runtime_error("Failed to get digest");
  }
  initialized = false;
  HMAC_CTX_cleanup(ctx);

  return std::move(make_intrusive_<Buffer>(out, size, EVP_MAX_MD_SIZE));
}
}  // namespace dsa