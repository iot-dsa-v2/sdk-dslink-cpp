#include "dsa/crypto.h"
#include "gtest/gtest.h"


using namespace dsa;


//---------------------------------------------
class CryptoTest {
public:
  CryptoTest(const char *curve_name) {
    int nid = OBJ_sn2nid(curve_name);
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

  return std::move(std::make_shared<Buffer>(out, size, size));
}


//---------------------------------------------
class BufferExt : public Buffer {
 public:
  BufferExt(const Buffer &other) {
    _size = other.size();
    _data = new uint8_t[_size];
    memcpy(_data, other.data(), _size);
  }
  
  void print() {
    uint8_t *ptr = _data;
    for(uint i=0; i<_size; ++i) {
      printf("%02x", *ptr++);
    }
    printf("\n");
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


TEST(HandshakeTest, KeyPairs) {
  CryptoTest ct("prime256v1");

  //Buffer other(public_key);
  std::string str_public_key("0415caf59c92efecb9253ea43912b419941fdb59a23d5d1289027128bf3d6ee4cb86fbe251b675a8d9bd991a65caa1bb23f8a8e0dd4eb0974f6b1eaa3436cec0e9");
  Buffer other;
  size_t size = str_public_key.size();
  other.assign((uint8_t *)str_public_key.c_str(), size);

  BufferPtr out = ct.compute_public_key("55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59");
  BufferExt outExt = BufferExt(*out);
  //--
  outExt.print();
  std::cout << str_public_key.c_str() << std::endl;
  //--
  std::cout << (outExt == other) << std::endl;

  //out = ct.compute_public_key("82848ef9d9204097a98a8c393e06aac9cb9a1ba3cdabf772f4ca7e6899b9f277");
  //print(out->data(), *out->get());


  ECDH ecdh("prime256v1");
  BufferPtr priv_key = ecdh.get_private_key();
  BufferPtr public_key = ecdh.get_public_key();
  out = ct.compute_public_key(priv_key->data(), priv_key->size());
  outExt = BufferExt(*out);
  // std::cout << (outExt == public_key) << std::endl;


  // BufferPtr out = ct.compute_public_key("55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59");
  // out = ct.compute_public_key("82848ef9d9204097a98a8c393e06aac9cb9a1ba3cdabf772f4ca7e6899b9f277");
}
