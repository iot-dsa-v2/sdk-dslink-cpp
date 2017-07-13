#include "hmac.h"

#include <utility>

#include <openssl/hmac.h>

namespace dsa {
void hmac::init(const char* alg, ByteBuffer& content) {
  const EVP_MD* md = EVP_get_digestbyname(alg);
  if (md == nullptr) throw std::runtime_error("Failed to initialize HMAC");

  // HMAC_CTX c;

  HMAC_CTX_init(&ctx);

  if (!HMAC_Init_ex(&ctx, content.data(), content.size(), md, nullptr))
    throw std::runtime_error("Failed to initialize HMAC");
}

hmac::hmac(const char* alg, ByteBuffer& data) {
  // ctx = HMAC_CTX_new();
  init(alg, data);
  initialized = true;
}

hmac::~hmac() {
  // HMAC_CTX_free(ctx);
}

void hmac::update(ByteBuffer& content) {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");
  int r = HMAC_Update(&ctx, content.data(), content.size());
  if (!r) throw std::runtime_error("Failed to update HMAC");
}

std::shared_ptr<ByteBuffer> hmac::digest() {
  if (!initialized) throw std::runtime_error("HMAC needs to be initialized");

  uint8_t* md_value = new uint8_t[EVP_MAX_MD_SIZE];
  unsigned int md_len = 0;

  bool r = HMAC_Final(&ctx, md_value, &md_len);
  if (!r) throw std::runtime_error("Failed to get digest");
  initialized = false;
  HMAC_CTX_cleanup(&ctx);

  return std::move(
      std::make_shared<ByteBuffer>(md_value, md_len, EVP_MAX_MD_SIZE));
}
}  // namespace dsa