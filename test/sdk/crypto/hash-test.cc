#include "dsa/crypto.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(HashTest, HashTypeInvalid) {
  EXPECT_THROW(
      {
        try {
          Hash hash("sha256-invalid");
        } catch (std::runtime_error &e) {
          EXPECT_STREQ("invalid hash type", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(HashTest, update__digest_base64CallTwice) {
  ECDH ecdh;
  std::vector<uint8_t> public_key = ecdh.get_public_key();

  Hash hash("sha256");
  hash.update(public_key);
  hash.digest_base64();

  EXPECT_THROW(
      {
        try {
          hash.digest_base64();
        } catch (std::runtime_error &e) {
          EXPECT_STREQ("digest already called", e.what());
          throw;
        }
      },
      std::runtime_error);
}
