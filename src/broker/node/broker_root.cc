#include "dsa_common.h"

#include "broker_root.h"

#include "../broker.h"
#include "../remote_node/remote_node_group.h"
#include "home/home_root.h"
#include "pub/pub_root.h"
#include "sys/sys_root.h"
#include "upstream/upstream_root.h"

namespace dsa {
BrokerRoot::BrokerRoot(const LinkStrandRef &strand, ref_<DsBroker> &&broker)
    : NodeModel(strand),
      _broker(std::move(broker)),
      _downstream_root(new RemoteNodeGroup(_strand->get_ref())) {
  add_list_child("Downstream", _downstream_root->get_ref());
  add_list_child("Upstream", new UpstreamRoot(_strand->get_ref()));
  add_list_child("Home", new BrokerHomeRoot(_strand->get_ref()));

  _sys.reset(new BrokerSysRoot(_strand->get_ref(), std::move(_broker)));
  add_list_child("Sys", _sys->get_ref());

  _pub.reset(new BrokerPubRoot(_strand->get_ref(), ""));
  add_list_child("Pub", _pub->get_ref());
}

BrokerRoot::~BrokerRoot() = default;
void BrokerRoot::destroy_impl() {
  // TODO: remove all children
}
}  // namespace dsa
