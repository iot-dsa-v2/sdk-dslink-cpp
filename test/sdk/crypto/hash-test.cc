#include "dsa/crypto.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(HashTest, HashTypeInvalid) {
  EXPECT_THROW({
      try {
	Hash hash("sha256-invalid");
      } catch (std::runtime_error &e) {
	EXPECT_STREQ("invalid hash type", e.what());
        throw;
      }
    }, std::runtime_error);
}

TEST(HashTest, update__digest_base64) {
  ECDH ecdh("prime256v1");
  BufferPtr public_key(ecdh.get_public_key());

  Hash hash("sha256");
  hash.update(*public_key);
  hash.digest_base64();

  EXPECT_THROW({
      try {
	hash.digest_base64();
      } catch (std::runtime_error &e) {
	EXPECT_STREQ("digest already called", e.what());
        throw;
      }
    }, std::runtime_error);
}

#if 0

// handshake message f0, client -> broker:
a70000000f00f00000000000000000020032006d796c696e6b2d54544458744c2d555f4e5132736746525535773048725a56696232442d4f3443785851724b6b34685573490415caf59c92efecb9253ea43912b419941fdb59a23d5d1289027128bf3d6ee4cb86fbe251b675a8d9bd991a65caa1bb23f8a8e0dd4eb0974f6b1eaa3436cec0e900c4ca4238a0b923820dcc509a6f75849bc81e728d9d4c2f636f067f89cc14862c

// handshake message f1, broker -> client:
a40000000f00f10000000000000000320062726f6b65722d67363735676153516f677a4d786a4a46764c374873436279533842304c79325f4162686b775f2d6734694904f9e64edcec5ea0a645bd034e46ff209dd9fb21d8aba74a5531dc6dcbea28d696c6c9386d924ebc2f48092a1d6c8b2ca907005cca7e8d2a58783b8a765d8eb29deccbc87e4b5ce2fe28308fd9f2a7baf3a87ff679a2f3e71d9181a67b7542122c

// handshake message f2, client -> broker:
470000000f00f20000000000000000130073616d706c655f746f6b656e5f737472696e67010100f58c10e212a82bf327a020679c424fc63e852633a53253119df74114fac8b2ba

// handshake message f3, broker -> client:
560000000f00f30000000000000000110073616d70652d73657373696f6e2d30303112002f646f776e73747265616d2f6d6c696e6b31e709059f1ebb84cfb8c34d53fdba7fbf20b1fe3dff8c343050d2b5c7c62be85a

#endif

