#include <sstream>

#include "dsa/crypto.h"
#include "dsa/message.h"
#include "gtest/gtest.h"

#include "util/little_endian.h"

using namespace dsa;

#define TO_CHAR(x) ((x) <= 0x9) ? (x) + 0x30 : 'a' + (x)-0xa

void to_char(std::stringstream& ss, uint8_t x) {
  ss << string_(1, TO_CHAR((x & 0xf0) >> 4));
  ss << string_(1, TO_CHAR(x & 0xf));
}

// Client Info
char client_private_key[] =
    "55e1bcad391b655f97fe3ba2f8e3031c9b5828b16793b7da538c2787c3a4dc59";

const unsigned char client_salt[] = {
    0xc4, 0xca, 0x42, 0x38, 0xa0, 0xb9, 0x23, 0x82, 0x0d, 0xcc, 0x50,
    0x9a, 0x6f, 0x75, 0x84, 0x9b, 0xc8, 0x1e, 0x72, 0x8d, 0x9d, 0x4c,
    0x2f, 0x63, 0x6f, 0x06, 0x7f, 0x89, 0xcc, 0x14, 0x86, 0x2c};

string_ client_token("sample_token_string");

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

  Hash hash;
  hash.update(client_public_key);

  message.dsid = "mylink-" + base64_url_convert(hash.digest_base64());

  // Client public _key
  message.public_key = client_public_key;

  // Client salt
  message.salt =
      std::vector<uint8_t>(client_salt, client_salt + sizeof(client_salt));

  // Update header_size and message_size
  message.size();

  //
  uint8_t buf[1024];
  message.write(buf);

  string_ expected_values(
      "9e0000000700f0020032006d796c696e6b2d54544458744c2d555f4e5132736746525535"
      "773048725a56696232442d4f3443785851724b6b34685573490415caf59c92efecb9253e"
      "a43912b419941fdb59a23d5d1289027128bf3d6ee4cb86fbe251b675a8d9bd991a65caa1"
      "bb23f8a8e0dd4eb0974f6b1eaa3436cec0e9c4ca4238a0b923820dcc509a6f75849bc81e"
      "728d9d4c2f636f067f89cc14862c");
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

  Hash hash;
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

  string_ expected_values(
      "9c0000000700f1320062726f6b65722d67363735676153516f677a4d786a4a46764c3748"
      "73436279533842304c79325f4162686b775f2d6734694904f9e64edcec5ea0a645bd034e"
      "46ff209dd9fb21d8aba74a5531dc6dcbea28d696c6c9386d924ebc2f48092a1d6c8b2ca9"
      "07005cca7e8d2a58783b8a765d8eb29deccbc87e4b5ce2fe28308fd9f2a7baf3a87ff679"
      "a2f3e71d9181a67b7542122c");
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

  dsa::HMAC hmac(client_shared_secret);

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

  string_ expected_values(
      "430000000700f2130073616d706c655f746f6b656e5f737472696e67010000000000"
      "00f58c10e212a82bf327a020679c424fc63e852633a53253119df74114fac8b2ba");
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

  // var brokerSharedSecret = brokerECDH.computeSecret(clientPublic);
  // var brokerAuth = crypto.createHmac('sha256',
  // brokerSharedSecret).update(clientSalt).digest();

  ECDH client_ecdh, broker_ecdh;
  client_ecdh.set_private_key_hex(client_private_key);
  broker_ecdh.set_private_key_hex(broker_private_key);
  std::vector<uint8_t> broker_shared_secret =
      broker_ecdh.compute_secret(client_ecdh.get_public_key());

  dsa::HMAC hmac(broker_shared_secret);

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

  string_ expected_values(
      "400000000700f3010000000012002f646f776e73747265616d2f6d6c696e6b31e709059f"
      "1ebb84cfb8c34d53fdba7fbf20b1fe3dff8c343050d2b5c7c62be85a");
  size_t length = expected_values.length();

  std::stringstream ss;
  for (uint8_t i = 0; i < length / 2; ++i) {
    to_char(ss, buf[i]);
  }

  EXPECT_EQ(expected_values, ss.str());
}
