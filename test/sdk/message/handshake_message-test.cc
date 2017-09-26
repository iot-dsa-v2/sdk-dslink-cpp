#include <sstream>

#include "dsa/crypto.h"
#include "dsa/message.h"
#include "gtest/gtest.h"

#include "util/little_endian.h"

using namespace dsa;

#define TO_CHAR(x) ((x) <= 0x9) ? (x) + 0x30 : 'a' + (x)-0xa

void to_char(std::stringstream& ss, uint8_t x) {
  ss << std::string(1, TO_CHAR((x & 0xf0) >> 4));
  ss << std::string(1, TO_CHAR(x & 0xf));
}

// Client Info
char client_private_key[] =
    "55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59";

const unsigned char client_salt[] = {
    0xc4, 0xca, 0x42, 0x38, 0xa0, 0xb9, 0x23, 0x82, 0x0d, 0xcc, 0x50,
    0x9a, 0x6f, 0x75, 0x84, 0x9b, 0xc8, 0x1e, 0x72, 0x8d, 0x9d, 0x4c,
    0x2f, 0x63, 0x6f, 0x06, 0x7f, 0x89, 0xcc, 0x14, 0x86, 0x2c};

std::string client_token("sample_token_string");

// Broker Info
char broker_private_key[] =
    "82848ef9d9204097a98a8c393e06aac9cb9a1ba3cdabf772f4ca7e6899b9f277";

const uint8_t broker_salt[] = {0xec, 0xcb, 0xc8, 0x7e, 0x4b, 0x5c, 0xe2, 0xfe,
                               0x28, 0x30, 0x8f, 0xd9, 0xf2, 0xa7, 0xba, 0xf3,
                               0xa8, 0x7f, 0xf6, 0x79, 0xa2, 0xf3, 0xe7, 0x1d,
                               0x91, 0x81, 0xa6, 0x7b, 0x75, 0x42, 0x12, 0x2c};

std::vector<uint8_t> broker_public_key;

TEST(MessageTest, HandshakeF0) {
  HandshakeF0Message message;

  // Client dsid
  ECDH client_ecdh;
  client_ecdh.set_private_key_hex(client_private_key);
  std::vector<uint8_t> client_public_key = client_ecdh.get_public_key();

  Hash hash("sha256");
  hash.update(client_public_key);

  message.dsid = "mylink-" + base64_url_convert(hash.digest_base64());

  // Client public _key
  message.public_key = client_public_key;

  // no encryption
  message.security_preference = false;

  // Client salt
  message.salt =
      std::vector<uint8_t>(client_salt, client_salt + sizeof(client_salt));

  // Update header_size and message_size
  message.size();

  //
  uint8_t buf[1024];
  message.write(buf);

  std::string expected_values(
      "a70000000f00f00000000000000000020032006d796c696e6b2d54544458744c2d555f4e"
      "5132736746525535773048725a56696232442d4f3443785851724b6b34685573490415ca"
      "f59c92efecb9253ea43912b419941fdb59a23d5d1289027128bf3d6ee4cb86fbe251b675"
      "a8d9bd991a65caa1bb23f8a8e0dd4eb0974f6b1eaa3436cec0e900c4ca4238a0b923820d"
      "cc509a6f75849bc81e728d9d4c2f636f067f89cc14862c");
  size_t length = expected_values.length();

  std::stringstream ss;
  for (uint8_t i = 0; i < length / 2; ++i) {
    to_char(ss, buf[i]);
  }

  EXPECT_EQ(expected_values, ss.str());
}

TEST(MessageTest, HandshakeF1) {
  HandshakeF1Message message;

  // Broker dsid
  ECDH broker_ecdh;
  broker_ecdh.set_private_key_hex(broker_private_key);
  std::vector<uint8_t> broker_public_key = broker_ecdh.get_public_key();

  Hash hash("sha256");
  hash.update(broker_public_key);

  message.dsid = "broker-" + base64_url_convert(hash.digest_base64());

  // Broker public _key
  message.public_key = broker_public_key;

  // Broker salt

  message.salt =
      std::vector<uint8_t>(broker_salt, broker_salt + sizeof(broker_salt));

  // Update header_size and message_size
  message.size();

  //
  uint8_t buf[1024];
  message.write(buf);

  std::string expected_values(
      "a40000000f00f10000000000000000320062726f6b65722d67363735676153516f677a4d"
      "786a4a46764c374873436279533842304c79325f4162686b775f2d6734694904f9e64edc"
      "ec5ea0a645bd034e46ff209dd9fb21d8aba74a5531dc6dcbea28d696c6c9386d924ebc2f"
      "48092a1d6c8b2ca907005cca7e8d2a58783b8a765d8eb29deccbc87e4b5ce2fe28308fd9"
      "f2a7baf3a87ff679a2f3e71d9181a67b7542122c");
  size_t length = expected_values.length();

  std::stringstream ss;
  for (uint8_t i = 0; i < length / 2; ++i) {
    to_char(ss, buf[i]);
  }

  EXPECT_EQ(expected_values, ss.str());
}

TEST(MessageTest, HandshakeF2) {
  HandshakeF2Message message;

  message.token = client_token;
  message.is_responder = true;

  ECDH client_ecdh, broker_ecdh;
  client_ecdh.set_private_key_hex(client_private_key);
  broker_ecdh.set_private_key_hex(broker_private_key);
  std::vector<uint8_t> client_shared_secret =
      client_ecdh.compute_secret(broker_ecdh.get_public_key());

  dsa::HMAC hmac("sha256", client_shared_secret);

  std::vector<uint8_t> salt_buffer =
      std::vector<uint8_t>(broker_salt, broker_salt + sizeof(broker_salt));
  hmac.update(salt_buffer);

  message.auth = hmac.digest();
  // TODO
  message.last_ack_id = 0;

  // Update header_size and message_size
  message.size();

  //
  uint8_t buf[1024];
  message.write(buf);

  std::string expected_values(
      "4b0000000f00f20000000000000000130073616d706c655f746f6b656e5f737472696e67"
      "01000000000000f58c10e212a82bf327a020679c424fc63e852633a53253119df74114fa"
      "c8b2b"
      "a");
  size_t length = expected_values.length();

  std::stringstream ss;
  for (uint8_t i = 0; i < length / 2; ++i) {
    to_char(ss, buf[i]);
  }

  EXPECT_EQ(expected_values, ss.str());
}

TEST(MessageTest, HandshakeF3) {
  HandshakeF3Message message;

  message.path = "/downstream/mlink1";
  message.session_id = "sampe-session-001";

  // var brokerSharedSecret = brokerECDH.computeSecret(clientPublic);
  // var brokerAuth = crypto.createHmac('sha256',
  // brokerSharedSecret).update(clientSalt).digest();

  ECDH client_ecdh, broker_ecdh;
  client_ecdh.set_private_key_hex(client_private_key);
  broker_ecdh.set_private_key_hex(broker_private_key);
  std::vector<uint8_t> broker_shared_secret =
      broker_ecdh.compute_secret(client_ecdh.get_public_key());

  dsa::HMAC hmac("sha256", broker_shared_secret);

  std::vector<uint8_t> salt_buffer =
      std::vector<uint8_t>(client_salt, client_salt + sizeof(client_salt));
  hmac.update(salt_buffer);

  message.auth = hmac.digest();

  // TODO
  message.allow_requester = true;
  message.last_ack_id = 0;

  // Update header_size and message_size
  message.size();

  //
  uint8_t buf[1024];
  message.write(buf);

  std::string expected_values(
      "5b0000000f00f3000000000000000001110073616d70652d73657373696f6e2d30303100"
      "0000001200"
      "2f646f776e73747265616d2f6d6c696e6b31e709059f1ebb84cfb8c34d53fdba7fbf20b1"
      "fe3dff8c343050d2b5c7c62be85a");
  size_t length = expected_values.length();

  std::stringstream ss;
  for (uint8_t i = 0; i < length / 2; ++i) {
    to_char(ss, buf[i]);
  }

  EXPECT_EQ(expected_values, ss.str());
}
