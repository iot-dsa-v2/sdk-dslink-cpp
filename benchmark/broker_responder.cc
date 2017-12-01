#include "dsa/responder.h"
#include "dslink.h"

using namespace dsa;
using namespace std;

// InvokeNodeModel

class BenchmarkNodeAction final : public InvokeNodeModel {
  explicit BenchmarkNodeAction(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {}

  void on_invoke(ref_<OutgoingInvokeStream>&& stream,
                 ref_<NodeState>& parent) final {
    stream->on_request([this](OutgoingInvokeStream& s,
                              ref_<const InvokeRequestMessage>&& message) {
      last_invoke_request = std::move(message);
    });
    auto response = make_ref_<InvokeResponseMessage>();
    response->set_value(Var("dsa"));
    stream->send_response(std::move(response));
  }
};

class BenchmarkNodeRoot final : public NodeModel {
 public:
  explicit BenchmarkNodeRoot(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    update_property("$invokable", "write");
    add_list_child("set_value", new BenchmarkNodeAction(_strand));
  };
};

int main(int argc, const char* argv[]) {
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  link->init_responder<BenchmarkNodeRoot>();
  // link->run();

  // add a callback when connected to broker
  link->run([&](const shared_ptr_<Connection> connection) {
    cout << endl << "connected to broker";

    // subscribe
    link->subscribe("child_a",
                    [](IncomingSubscribeCache&,
                       ref_<const SubscribeResponseMessage>& message) {
                      cout << endl
                           << "receive subscribe response" << endl
                           << message->get_value().value.to_string();
                    });

    // list on the root node
    link->list(
        "", [](IncomingListCache& cache, const std::vector<string_> changes) {
          cout << endl << "receive list response";
          auto& map = cache.get_map();
          for (auto& it : map) {
            cout << endl << it.first << " : " << it.second.to_json();
          }
        });
  });
  return 0;
}
