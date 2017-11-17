#include "dsa_common.h"

#include "broker_root.h"

#include "downstream/downstream_root.h"
#include "home/home_root.h"
#include "pub/pub_root.h"
#include "sys/sys_root.h"
#include "upstream/upstream_root.h"

namespace dsa {
BrokerRoot::BrokerRoot(LinkStrandRef &&strand) : NodeModel(std::move(strand)) {
  add_list_child("downstream", new DownstreamRoot(_strand->get_ref()));
  add_list_child("upstream", new UpstreamRoot(_strand->get_ref()));
  add_list_child("home", new HomeRoot(_strand->get_ref()));
  add_list_child("sys", new SysRoot(_strand->get_ref()));
  add_list_child("pub", new PubRoot(_strand->get_ref()));
}
}
