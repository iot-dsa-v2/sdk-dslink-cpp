#include "hmac.h"

namespace dsa {
void hmac::init(const char* alg, Buffer& content) {
  const EVP_MD* md = EVP_get_digestbyname(alg);
  if (md == nullptr) throw std::runtime_error("Failed to initialize HMAC");

  HMAC_CTX_init(&ctx);

  if (!HMAC_Init_ex(&ctx, content.data(), content.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");
}

hmac::hmac(const char* alg, Buffer& data) {
  init(alg, data);
  initialized = true;
}

hmac::~hmac() {
  // NOTE: to be used later if different version of OpenSSL is used
}

void hmac::update(Buffer& content) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");
  int r = HMAC_Update(&ctx, content.data(), content.size());
  if (!r) throw std::runtime_error("Failed to update HMAC");
}

BufferPtr hmac::digest() {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");

  uint8_t* out = new uint8_t[EVP_MAX_MD_SIZE];
  unsigned int size = 0;

  bool r = HMAC_Final(&ctx, out, &size);
  if (!r) {
    delete[] out;
    throw std::runtime_error("Failed to get digest");
  }
  initialized = false;
  HMAC_CTX_cleanup(&ctx);

  return std::move(
      std::make_shared<Buffer>(out, size, EVP_MAX_MD_SIZE));
}
}  // namespace dsa