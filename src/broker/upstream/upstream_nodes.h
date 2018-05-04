#ifndef DSA_SDK_UPSTREAM_NODES_H
#define DSA_SDK_UPSTREAM_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "module/storage.h"
#include "responder/node_model.h"

namespace dsa {
class Client;
class UpstreamConnectionNode;
class ValueNodeModel;

class UpstreamRootNode : public NodeModel {
  friend class UpstreamConnectionNode;

 protected:
  ref_<StrandStorageBucket> _storage;
  void destroy_impl() override;
  void initialize() override;

 public:
  UpstreamRootNode(const LinkStrandRef &strand);
};

class UpstreamConnectionNode : public NodeModel {
  ref_<Client> _client;
  ref_<UpstreamRootNode> _parent;

  bool _enabled = true;
  string_ _name;
  string_ _url;
  string_ _token;
  string_ _role;

  ref_<ValueNodeModel> _enabled_node;
  ref_<ValueNodeModel> _name_node;
  ref_<ValueNodeModel> _url_node;
  ref_<ValueNodeModel> _token_node;
  ref_<ValueNodeModel> _role_node;

 public:
  UpstreamConnectionNode(const LinkStrandRef &strand,
                         ref_<UpstreamRootNode> &&parent,
                         ref_<NodeModel> &&profile);
  ~UpstreamConnectionNode();

 protected:
  void destroy_impl() override;

  void save_upstream() const;
  void save_extra(VarMap &map) const override;
  void load_extra(VarMap &map) override;

  void connection_changed();
};

}  // namespace dsa

#endif  // DSA_SDK_UPSTREAM_NODES_H
