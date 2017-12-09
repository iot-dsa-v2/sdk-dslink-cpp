#include "dsa/responder.h"
#include "dslink.h"

using namespace dsa;
using namespace std;

class ExampleNodeChild : public NodeModel {
 public:
  explicit ExampleNodeChild(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    update_property("$type", Var("string"));
    update_property("@attr", Var("test attribute value"));
    set_value(Var("test string value 1"));
  };
};

class ExampleNodeRoot : public NodeModel {
 public:
  explicit ExampleNodeRoot(LinkStrandRef strand)
      : NodeModel(std::move(strand)) {
    add_list_child("child_a", new ExampleNodeChild(_strand));
    add_list_child("child_b", new ExampleNodeChild(_strand));
  };
};

int main(int argc, const char* argv[]) {
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  link->init_responder_main<ExampleNodeRoot>();
  // link->run();

  // add a callback when connected to broker
  link->run([&](const shared_ptr_<Connection> connection) {
    cout << endl << "connected to broker";

    // subscribe
    link->subscribe("main/child_a",
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
