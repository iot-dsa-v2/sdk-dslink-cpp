#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/requester.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include <broker.h>
#include <config/broker_config.h>
#include <gtest/gtest.h>
#include <module/default/console_logger.h>
#include <util/string.h>
#include "../sdk/async_test.h"
#include "../sdk/test_config.h"
#include "module/logger.h"

using namespace dsa;

namespace broker_dslink_test {

class MockNodeChild : public NodeModel {
 public:
  explicit MockNodeChild(const LinkStrandRef &strand);
};

class MockNodeRoot : public NodeModel {
 public:
  bool need_list();
  explicit MockNodeRoot(const LinkStrandRef &strand);
};

ref_<DsBroker> create_broker(std::shared_ptr<App> app = nullptr);
ref_<DsLink> create_dslink(
    std::shared_ptr<App> app, int port, string_ dslink_name,
    bool connect = false,
    dsa::ProtocolType protocol = dsa::ProtocolType::PROT_DS);
ref_<DsLink> create_mock_dslink(
    std::shared_ptr<App> app, int port, string_ dslink_name,
    bool connect = false,
    dsa::ProtocolType protocol = dsa::ProtocolType::PROT_DS);
}
