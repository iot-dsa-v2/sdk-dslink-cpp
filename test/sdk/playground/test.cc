#include <broker.h>
#include <config/broker_config.h>
#include <config/module_loader.h>
#include <module/default/console_logger.h>
#include <util/string.h>
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/requester.h"
#include "dsa/message.h"
#include "dsa/stream.h"

#include "../async_test.h"
#include "../test_config.h"
#include "gtest/gtest.h"
#include "module/logger.h"

using namespace dsa;

class MockNodeAction : public InvokeNodeModel {
 public:
  explicit MockNodeAction(LinkStrandRef strand)
      : InvokeNodeModel(std::move(strand)){};

  void on_invoke(ref_<OutgoingInvokeStream>&& stream,
                 ref_<NodeState>& parent) final {
      stream->on_request([this](OutgoingInvokeStream& s,
                                ref_<const InvokeRequestMessage>&& msg) {
        if (msg != nullptr) {
            auto response = make_ref_<InvokeResponseMessage>();
            response->set_value(Var((msg->get_value().to_string() + " world")));
            response->set_status(MessageStatus::CLOSED);
            s.send_response(std::move(response));
        } else {
            // nullptr message means stream closed
        }
      });
  }
};

class MockNodeValue : public NodeModel {
 public:
  explicit MockNodeValue(LinkStrandRef strand) : NodeModel(std::move(strand)) {
      set_value(Var("hello world"));
  };
};
class MockNodeRoot : public NodeModel {
 public:
  explicit MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
      add_list_child("value", make_ref_<MockNodeValue>(_strand));

      // add a child action
      auto node = make_ref_<NodeModel>(_strand->get_ref());
      add_list_child("node", node->get_ref());
      node->add_list_child("action", make_ref_<MockNodeAction>(_strand));
  };
};


ref_<DsLink> create_dslink(std::shared_ptr<App> app, int port, string_ dslink_name) {
    std::string address =
        std::string("127.0.0.1:") + std::to_string(port);

    const char *argv[] = {"./test", "-b", address.c_str()};
    int argc = 3;
    auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
    link->init_responder();

    return link;
}

ref_<DsBroker> create_broker() {
    const char* empty_argv[1];
    ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(0, empty_argv);
    ModuleLoader modules(broker_config);
    auto broker = make_ref_<DsBroker>(std::move(broker_config), modules);

    return std::move(broker);
}


TEST(PlayTest, SESSION) {
    auto broker = create_broker();
    shared_ptr_<App> &app = broker->get_app();
	broker->strand->logger().level = Logger::ALL___;

    auto link_1 = create_dslink(app, broker->tcp_server_port, "test_1");
	link_1->strand->logger().level = Logger::ALL___;
    auto link_2 = create_dslink(app, broker->tcp_server_port, "test_2");
	link_2->strand->logger().level = Logger::ALL___;

    int step = 0;
// when list on downstream/test1 it should have a metadata for test1's dsid
    auto unavailable_child_list = [&](const shared_ptr_<Connection> &connection) {
      link_1->list(
          "downstream/test_2",
          [&](IncomingListCache &cache, const std::vector<string_>) {
            step++;
            switch (step) {
                case 1: {
                    // step 1, connect client 2
                    std::cout<<"Step 1"<<std::endl;
                    EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
                    link_2->connect();
                    break;
                }
                case 2: {
                    // step 2, disconnect client 2
                    std::cout<<"Step 2"<<std::endl;
                    EXPECT_EQ(cache.get_status(), MessageStatus::OK);
                    link_2->strand->post([link_2]() {
                      link_2->destroy();
                    });
                    break;
                }

                default: {  //   case 3:{
                    // step 3, end test
                    std::cout<<"Step 3"<<std::endl;
                    EXPECT_EQ(cache.get_status(), MessageStatus::NOT_AVAILABLE);
                    link_1->strand->post([link_1]() {
                      link_1->destroy();
                    });
                    broker->strand->post([broker]() { broker->destroy(); });
                }
            }

          });
    };

    link_1->connect(std::move(unavailable_child_list));

    broker->run();
    EXPECT_TRUE(broker->is_destroyed());
}


TEST(PlayTest, CLOSE) {
    std::string close_token = "12345678901234567890123456789012";
    string_to_file(close_token, ".close_token");

    auto broker = create_broker();
    shared_ptr_<App> &app = broker->get_app();

    auto link_1 = create_dslink(app, broker->tcp_server_port, "test_1");
    auto link_2 = create_dslink(app, broker->tcp_server_port, "test_2");

    link_1->connect([&](const shared_ptr_<Connection> connection) {
      link_2->connect([&](const shared_ptr_<Connection> connection) {

//      link_2->list("downstream/test_1/sys",
//                     [&](IncomingListCache &cache2, const std::vector<string_> &str2) {
//                         std::cout<<"Listed"<<std::endl;
//                       auto map = cache2.get_map();
//                       if(map["stop"].get_type() != 0)
//                         std::cout<<"STOP EXISTS"<<std::endl;
//                     });
      WAIT(1000);

        ref_<InvokeRequestMessage> invoke_req = make_ref_<InvokeRequestMessage>();
        invoke_req->set_value(Var(close_token));
        invoke_req->set_target_path("downstream/test_1/sys/stop");

        link_2->invoke(
            [&](IncomingInvokeStream &stream, ref_<const InvokeResponseMessage> &&msg) {
              //no response here!!!
              std::cout<<"Close Invoked"<<std::endl;
              broker->strand->post([broker]() { broker->destroy(); });
              link_2->strand->post([link_2]() { link_2->destroy(); });
            },
            std::move(invoke_req));

        WAIT(1000);
      });
    });


    broker->run();
    EXPECT_TRUE(broker->is_destroyed());
}