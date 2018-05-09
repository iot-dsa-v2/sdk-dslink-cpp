#ifndef DSA_SDK_UPSTREAM_MANAGER_H
#define DSA_SDK_UPSTREAM_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "core/link_strand.h"
#include "util/enable_ref.h"

namespace dsa {

class Client;
class NodeModel;
class BrokerPubRoot;
class UpstreamRootNode;

class UpstreamManager : public DestroyableRef<UpstreamManager> {
 protected:
  LinkStrandRef _strand;

  ref_<UpstreamRootNode> _upstream_root;


  void destroy_impl() override;

 public:
  explicit UpstreamManager(const LinkStrandRef& strand);
  ~UpstreamManager();

  void add_node(NodeModel& sys_node, BrokerPubRoot& pub_root);
};

}  // namespace dsa

#endif  // DSA_SDK_UPSTREAM_MANAGER_H
