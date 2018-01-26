#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/editable_strand.h"
#include "dslink.h"

#include <gtest/gtest.h>
#include <string>

namespace dsa {

class App;
class Client;

enum class ProtocolType : uint8_t {
  PROT_DS = 0x00,
  PROT_DSS = 0x10,
  PROT_WS = 0x01,
  PROT_WSS = 0x11,
};

class TestConfig : public WrapperStrand {
 private:
  std::shared_ptr<App> app;

 public:
  explicit TestConfig(std::shared_ptr<App>& app, bool async = false);

  WrapperStrand get_client_wrapper_strand();

  std::shared_ptr<TcpServer> create_server();
  ref_<DsLink> create_dslink(bool async = true);
};

void destroy_client_in_strand(ref_<Client>& client);
void destroy_dslink_in_strand(ref_<DsLink>& dslink);
}

class SetUpBase : public ::testing::Test {
 protected:
  SetUpBase() : _protocol(dsa::ProtocolType::PROT_DS) {}

  virtual void SetUp() {
    const char* protocol = std::getenv("COM_PROTOCOL");
    if (protocol == nullptr) return;

    if (!strcmp(protocol, "dss")) {
      _protocol = dsa::ProtocolType::PROT_DSS;
    } else if (!strcmp(protocol, "ws")) {
      _protocol = dsa::ProtocolType::PROT_WS;
    } else if (!strcmp(protocol, "wss")) {
      _protocol = dsa::ProtocolType::PROT_WSS;
    }
  }

  // virtual void TearDown() {}

  const dsa::ProtocolType protocol() const { return _protocol; }

 private:
  dsa::ProtocolType _protocol;
};

#endif  // PROJECT_TEST_MODULES_H
