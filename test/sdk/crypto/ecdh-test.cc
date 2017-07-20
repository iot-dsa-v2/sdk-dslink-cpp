#include "dsa/crypto.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(ECDHTest, CurveUndef) {
  EXPECT_THROW(ECDH("secp256k1-undef"), std::runtime_error);
}

TEST(ECDHTest, CurveUndefExMsg) {
  EXPECT_THROW({
      try {
	ECDH("secp256k1-undef");
      } catch (std::runtime_error &e) {
	EXPECT_STREQ("invalid curve name", e.what());
        throw;
      }
    }, std::runtime_error);
}

TEST(ECDHTest, get_private_key) {
  ECDH ecdh1("secp256k1");
  ECDH ecdh2("secp256k1");

  BufferPtr pkey1 = ecdh1.get_private_key();
  BufferPtr pkey2 = ecdh2.get_private_key();

  size_t pkey1_size = pkey1->size();
  EXPECT_EQ(32, pkey1_size);
  EXPECT_EQ(32, pkey2->size());

  EXPECT_NE(0, strncmp((const char *)pkey1->data(), 
		       (const char*)pkey2->data(), pkey1_size));
}

