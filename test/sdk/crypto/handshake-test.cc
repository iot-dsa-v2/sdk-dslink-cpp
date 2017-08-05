#include "dsa/crypto.h"
#include "gtest/gtest.h"
#include <boost/format.hpp>


using namespace dsa;
using boost::format;


//---------------------------------------------
class CryptoTest {
public:
  CryptoTest() {
    int nid = OBJ_sn2nid(ECDH::curve_name);
    _key = EC_KEY_new_by_curve_name(nid);
    _group = EC_KEY_get0_group(_key);
  }
  ~CryptoTest() {
    EC_KEY_free(_key);
  }

  //
  BufferPtr compute_public_key(const char *priv_key);
  BufferPtr compute_public_key(uint8_t *priv_key, size_t size);
  BufferPtr compute_public_key(BIGNUM *priv);

private:
  EC_KEY *_key;
  const EC_GROUP *_group;
};


BufferPtr CryptoTest::compute_public_key(const char *priv_key) {
  BIGNUM bn;
  BIGNUM *priv;

  BN_init(&bn);
  priv = &bn;
  BN_hex2bn(&priv, priv_key);

  BufferPtr out = compute_public_key(priv);
  BN_free(priv);

  return out;
}


BufferPtr CryptoTest::compute_public_key(uint8_t *priv_key, size_t size) {
  BIGNUM bn;
  BIGNUM *priv;

  BN_init(&bn);
  priv = &bn;
  BN_bin2bn(priv_key, size, priv);

  BufferPtr out = compute_public_key(priv);

  BN_free(priv);

  return out;
}


BufferPtr CryptoTest::compute_public_key(BIGNUM *priv) {
  EC_POINT *pub_key = nullptr;
  pub_key = EC_POINT_new(_group);


  if (!EC_POINT_mul(_group, pub_key, priv, nullptr, nullptr, nullptr))
    std::cerr << "EC_POINT_mul error!" << std::endl;

  EC_KEY_set_public_key(_key, pub_key);
  point_conversion_form_t form = EC_GROUP_get_point_conversion_form(_group);

  size_t size = EC_POINT_point2oct(_group, pub_key, form, nullptr, 0, nullptr);

  uint8_t *out = new uint8_t[size];
  EC_POINT_point2oct(_group, pub_key, form, out, size, nullptr);
  EC_POINT_free(pub_key);

  return std::move(make_intrusive_<Buffer>(out, size, size));
}


//---------------------------------------------
class BufferExt : public Buffer {
 public:
  BufferExt(const Buffer &other) {
    _size = other.size();
    _data = new uint8_t[_size];
    memcpy(_data, other.data(), _size);
  }
  
  std::string hexstr() {
    uint8_t *ptr = _data;
    boost::format formater("%02x");
    std::string out;
    for(uint32_t i=0; i<_size; ++i) {
      formater % (int)_data[i];
      out += formater.str();
    }
    return out;
  }

  bool operator==(const BufferExt &other) {
    return (memcmp(_data, other.data(), _size) == 0);
  }

  ~BufferExt() {
    delete[] _data;
  }

 private:
  size_t _size;
  uint8_t* _data;
};


//---------------------------------------------
TEST(HandshakeTest, ClientInfo) {
  char client_private_key[] = "55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59";

  CryptoTest ct;

  // Keys
  BufferPtr public_key = ct.compute_public_key(client_private_key);
  BufferExt public_key_ext(*public_key);

  //Buffer other(other_public_key);
  std::string expected_public_key("0415caf59c92efecb9253ea43912b419941fdb59a23d5d1289027128bf3d6ee4cb86fbe251b675a8d9bd991a65caa1bb23f8a8e0dd4eb0974f6b1eaa3436cec0e9");
  EXPECT_EQ(expected_public_key, public_key_ext.hexstr());

  // DsId
  ECDH ecdh;
  ecdh.set_private_key_hex(client_private_key);

  Hash hash("sha256");
  hash.update(*ecdh.get_public_key());

  EXPECT_EQ("TTDXtL-U_NQ2sgFRU5w0HrZVib2D-O4CxXQrKk4hUsI", base64url(hash.digest_base64()));

  // Shared secret
  char server_private_key[] = "82848ef9d9204097a98a8c393e06aac9cb9a1ba3cdabf772f4ca7e6899b9f277";

  ECDH other_ecdh;
  other_ecdh.set_private_key_hex(server_private_key);
  BufferPtr shared_secret = ecdh.compute_secret(*other_ecdh.get_public_key());

  EXPECT_EQ("5f67b2cb3a0906afdcf5175ed9316762a8e18ce26053e8c51b760c489343d0d1",
	    BufferExt(*shared_secret).hexstr());
 
  // Auth
  // const char server_salt[] = "eccbc87e4b5ce2fe28308fd9f2a7baf3a87ff679a2f3e71d9181a67b7542122c";
  const unsigned char server_salt[] = {0xec, 0xcb, 0xc8, 0x7e, 0x4b, 0x5c, 0xe2, 0xfe,
                              0x28, 0x30, 0x8f, 0xd9, 0xf2, 0xa7, 0xba, 0xf3,
                              0xa8, 0x7f, 0xf6, 0x79, 0xa2, 0xf3, 0xe7, 0x1d,
			      0x91, 0x81, 0xa6, 0x7b, 0x75, 0x42, 0x12, 0x2c};

  Buffer server_salt_buffer;
  server_salt_buffer.assign((uint8_t*) server_salt, sizeof(server_salt));

  dsa::HMAC hmac("sha256", *shared_secret);
  hmac.update(server_salt_buffer);

  EXPECT_EQ("f58c10e212a82bf327a020679c424fc63e852633a53253119df74114fac8b2ba",
	    BufferExt(*hmac.digest()).hexstr());
}


TEST(HandshakeTest, ServerInfo) {
  char curve_name[] = "prime256v1";
  char server_private_key[] = "82848ef9d9204097a98a8c393e06aac9cb9a1ba3cdabf772f4ca7e6899b9f277";

  CryptoTest ct;

  // Keys
  BufferPtr public_key = ct.compute_public_key(server_private_key);
  BufferExt public_key_ext(*public_key);

  //Buffer other(other_public_key);
  std::string expected_public_key("04f9e64edcec5ea0a645bd034e46ff209dd9fb21d8aba74a5531dc6dcbea28d696c6c9386d924ebc2f48092a1d6c8b2ca907005cca7e8d2a58783b8a765d8eb29d");
  EXPECT_EQ(expected_public_key, public_key_ext.hexstr());

  // DsId
  ECDH ecdh;
  ecdh.set_private_key_hex(server_private_key);

  Hash hash("sha256");
  hash.update(*ecdh.get_public_key());

  EXPECT_EQ("g675gaSQogzMxjJFvL7HsCbyS8B0Ly2_Abhkw_-g4iI", base64url(hash.digest_base64()));

  // Shared secret
  char client_private_key[] = "55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59";
  ECDH other_ecdh;
  other_ecdh.set_private_key_hex(client_private_key);
  BufferPtr shared_secret = ecdh.compute_secret(*other_ecdh.get_public_key());

  EXPECT_EQ("5f67b2cb3a0906afdcf5175ed9316762a8e18ce26053e8c51b760c489343d0d1",
  	    BufferExt(*shared_secret).hexstr());

  // Server Auth
  // const char client_salt[] = "c4ca4238a0b923820dcc509a6f75849bc81e728d9d4c2f636f067f89cc14862c";
  const unsigned char client_salt[] = {0xc4, 0xca, 0x42, 0x38, 0xa0, 0xb9, 0x23, 0x82,
				       0x0d, 0xcc, 0x50, 0x9a, 0x6f, 0x75, 0x84, 0x9b,
				       0xc8, 0x1e, 0x72, 0x8d, 0x9d, 0x4c, 0x2f, 0x63,
				       0x6f, 0x06, 0x7f, 0x89, 0xcc, 0x14, 0x86, 0x2c};
  dsa::Buffer client_salt_buffer;
  client_salt_buffer.assign((uint8_t*) client_salt, sizeof(client_salt));

  dsa::HMAC hmac("sha256", *shared_secret);
  hmac.update(client_salt_buffer);

  EXPECT_EQ("e709059f1ebb84cfb8c34d53fdba7fbf20b1fe3dff8c343050d2b5c7c62be85a",
	    BufferExt(*hmac.digest()).hexstr());
}


TEST(HandShakeTest, HMAC) {
  const char key[] = "key";
  dsa::Buffer key_buffer;
  key_buffer.assign((uint8_t*)key, 3);

  const char message[] = "The quick brown fox jumps over the lazy dog";
  dsa::Buffer message_buffer;
  message_buffer.assign((const uint8_t*)message, 43);

  dsa::HMAC hmac("sha256", key_buffer);
  hmac.update(message_buffer);

  BufferPtr auth_message = hmac.digest();

  EXPECT_EQ("f7bc83f430538424b13298e6aa6fb143ef4d59a14946175997479dbc2d1a3cd8",
	    BufferExt(*auth_message).hexstr());

}


