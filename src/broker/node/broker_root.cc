#include "dsa_common.h"

#include "broker_root.h"

#include "../broker.h"
#include "../remote_node/remote_node_group.h"
#include "home/home_root.h"
#include "pub/pub_root.h"
#include "sys/sys_root.h"

namespace dsa {
BrokerRoot::BrokerRoot(const LinkStrandRef &strand, ref_<DsBroker> &&broker)
    : NodeModel(strand),
      _broker(std::move(broker)),
      _downstream_root(new RemoteNodeGroup(_strand)),
      _upstream_root(new RemoteNodeGroup(_strand)) {
  add_list_child("downstream", _downstream_root->get_ref());
  add_list_child("upstream", _upstream_root->get_ref());
  add_list_child("home", new BrokerHomeRoot(_strand));

  _sys.reset(new BrokerSysRoot(_strand, std::move(_broker)));
  add_list_child("sys", _sys->get_ref());

  _pub.reset(new BrokerPubRoot(_strand, ""));
  add_list_child("pub", _pub->get_ref());
}

BrokerRoot::~BrokerRoot() = default;
void BrokerRoot::destroy_impl() {
  // TODO: remove all children
}
}  // namespace dsa
