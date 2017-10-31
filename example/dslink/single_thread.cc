#include "dsa/responder.h"
#include "dslink.h"

using namespace dsa;

class ExampleNodeChild : public NodeModel {
 public:
  explicit ExampleNodeChild(LinkStrandRef strand)
      : NodeModel(std::move(strand)){

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

int main(int argc, const char *argv[]) {
  DsLink link(argc, argv, "mydslink", "1.0.0");
  link.init_responder<ExampleNodeRoot>();
  link.run();

  // // add a callback when connected to broker
  //  link.run([](const ClientConnetionData& data){
  //    std::cout << std::endl << "connected to broker";
  //  });
}
