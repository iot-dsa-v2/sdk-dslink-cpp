#include "dsa/crypto.h"
#include <gtest/gtest.h>

#include <boost/format.hpp>

using boost::format;

using namespace dsa;

TEST(ECDHTest, get_private_key) {
  ECDH ecdh1;
  ECDH ecdh2;

  std::vector<uint8_t> pkey1 = ecdh1.get_private_key();
  std::vector<uint8_t> pkey2 = ecdh2.get_private_key();

  size_t pkey1_size = pkey1.size();
  EXPECT_EQ(32, pkey1_size);
  EXPECT_EQ(32, pkey2.size());

  EXPECT_NE(0, memcmp(pkey1.data(), pkey2.data(), pkey1_size));
}

// TEST(ECDHTest, get_private_key_MemoryLeak) {
//  ECDH ecdh();
//  BytesPtr key(ecdh.get_private_key());
//  EXPECT_EQ(0, 0);
//}

TEST(ECDHTest, get_public_key) {
  ECDH ecdh1;
  ECDH ecdh2;

  std::vector<uint8_t> pkey1 = ecdh1.get_public_key();
  std::vector<uint8_t> pkey2 = ecdh2.get_public_key();

  size_t pkey1_size = pkey1.size();
  EXPECT_EQ(65, pkey1_size);
  EXPECT_EQ(65, pkey2.size());

  EXPECT_NE(0, memcmp((const char *)pkey1.data(), (const char *)pkey2.data(),
                      pkey1_size));
}

// TEST(ECDHTest, get_public_key_MemoryLeak) {
//  ECDH ecdh();
//  BytesPtr key(ecdh.get_public_key());
//  EXPECT_EQ(0, 0);
//}

TEST(ECDHTest, set_private_key_hex_MemoryLeak) {
  ECDH ecdh;

  const char data[128] = "0123456789abcdefABCDEF";
  ecdh.set_private_key_hex(data);
}

TEST(ECDHTest, set_private_key_hex_Exception) {
  ECDH ecdh;

  EXPECT_THROW(
      {
        try {
          const char data[128] = {0};
          ecdh.set_private_key_hex(data);
        } catch (std::runtime_error &e) {
          EXPECT_STREQ("invalid key for curve", e.what());
          throw;
        }
      },
      std::runtime_error);
}

TEST(ECDHTest, compute_secret) {
  ECDH A_ecdh;
  ECDH B_ecdh;

  std::vector<uint8_t> A_public_key = A_ecdh.get_public_key();
  std::vector<uint8_t> B_public_key = B_ecdh.get_public_key();

  std::vector<uint8_t> A_shared_secret = A_ecdh.compute_secret(B_public_key);
  std::vector<uint8_t> B_shared_secret = B_ecdh.compute_secret(A_public_key);

  size_t A_ss_size = A_shared_secret.size();
  size_t B_ss_size = B_shared_secret.size();

  EXPECT_EQ(B_ss_size, A_ss_size);
  EXPECT_EQ(0,
            memcmp(A_shared_secret.data(), B_shared_secret.data(), A_ss_size));
}
