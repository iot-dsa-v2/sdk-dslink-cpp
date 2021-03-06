
#include "broker_dslink_util.h"

using BrokerDsLinkTest = SetUpBase;

TEST_F(BrokerDsLinkTest, ProfileActionTest) {
  // First Create Broker
  auto app = make_shared_<App>();
  auto broker = broker_dslink_test::create_broker(app);
  broker->run();
  ASYNC_EXPECT_TRUE(1000, *broker->strand,
                    [&]() { return broker->get_active_server_port() != 0; });
  int32_t port;

  switch (protocol()) {
    case dsa::ProtocolType::PROT_DSS:
      port = broker->get_active_secure_port();
      break;
    default:
      port = broker->get_active_server_port();
  }

  EXPECT_TRUE(port != 0);

  auto link =
      broker_dslink_test::create_dslink(app, port, "test1", false, protocol());

  ref_<NodeModel> profile_example = make_ref_<NodeModel>(link->strand);
  profile_example->add_list_child(
      "change", make_ref_<SimpleInvokeNode>(
                    link->strand, [&](Var &&v, SimpleInvokeNode &node,
                                      OutgoingInvokeStream &stream,
                                      ref_<NodeState> &&parent) {
                      auto *parent_model = parent->model_cast<NodeModel>();
                      if (parent_model != nullptr) {
                        parent_model->set_value(std::move(v));
                      }
                      stream.close();
                    }));
  link->add_to_pub("example", profile_example->get_ref());

  ref_<NodeModel> main_node =
      make_ref_<NodeModel>(link->strand, profile_example->get_ref());
  link->init_responder(std::move(main_node));

  bool list_checked = false;
  bool invoked = false;
  bool subscrib_checked = false;
  ref_<IncomingListCache> list_cache;
  link->connect([&](const shared_ptr_<Connection> connection,
                    ref_<DsLinkRequester> link_req) {

    // check the list result
    link_req->list("downstream/test1/main",
                   [&](IncomingListCache &cache, const std::vector<string_> &) {
                     if (cache.get_map().count("$is") > 0 &&
                         cache.get_map().at("$is").to_string() == "example") {
                       EXPECT_TRUE(cache.get_profile_map().size() != 0);
                       EXPECT_NE(cache.get_profile_map().find("change"),
                                 cache.get_profile_map().end());
                       list_checked = true;
                       cache.close();
                     }
                   });
    // invoke the pub node to change the value
    auto request = make_ref_<InvokeRequestMessage>();
    request->set_target_path("downstream/test1/main/change");
    request->set_body(Var("hello").to_msgpack());
    link_req->invoke(
        CAST_LAMBDA(IncomingInvokeStreamCallback)[&](
            IncomingInvokeStream &, ref_<const InvokeResponseMessage> && msg) {
          EXPECT_EQ(msg->get_status(), Status::DONE);
          invoked = true;
        },
        std::move(request));
    // subscribe to check the result
    ref_<IncomingSubscribeCache> sub_cache;
    sub_cache = link_req->subscribe(
        "downstream/test1/main",
        [&](IncomingSubscribeCache &cache,
            ref_<const SubscribeResponseMessage> &msg) {
          if (msg->get_value().value.to_string() == "hello") {
            subscrib_checked = true;
            cache.close();
          }
        });

  });

  WAIT_EXPECT_TRUE(1000, [&]() -> bool {
    return list_checked && invoked && subscrib_checked;
  });
  destroy_dslink_in_strand(link);
  broker->strand->post([&]() { broker->destroy(); });

  app->close();

  WAIT_EXPECT_TRUE(1000, [&]() -> bool { return app->is_stopped(); });

  if (!app->is_stopped()) {
    app->force_stop();
  }
  app->wait();
}
