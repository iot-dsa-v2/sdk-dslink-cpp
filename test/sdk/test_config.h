#ifndef DSA_TEST_MODULES_H
#define DSA_TEST_MODULES_H

#include "core/editable_strand.h"
#include "dslink.h"
#include "web_server/web_server.h"

#include "network/ws/ws_connection.h"

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
  dsa::ProtocolType protocol;

  //  shared_ptr_<tcp::socket> _tcp_socket;
  shared_ptr_<boost::asio::ssl::context> _ssl_context;
  //  shared_ptr_<websocket_ssl_stream> _wss_stream;
  websocket_ssl_stream* _wss_stream;

 public:
  explicit TestConfig(std::shared_ptr<App>& app, bool async = false,
                      dsa::ProtocolType protocol = dsa::ProtocolType::PROT_DS);

  WrapperStrand get_client_wrapper_strand();

  std::shared_ptr<TcpServer> create_server();
  std::shared_ptr<WebServer> create_webserver();
  ref_<DsLink> create_dslink(bool async = true);
};

void destroy_client_in_strand(ref_<Client>& client);
void destroy_dslink_in_strand(ref_<DsLink>& dslink);
}

class SetUpBase : public ::testing::Test {
 protected:
  SetUpBase() : _protocol(dsa::ProtocolType::PROT_DS) {}

  virtual void SetUp() {
    const char* protocol = std::getenv("DSA_TEST_PROTOCOL");
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
