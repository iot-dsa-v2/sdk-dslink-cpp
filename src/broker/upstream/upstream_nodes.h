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
class UpstreamManager;
class RemoteRootNode;

class UpstreamRootNode : public NodeModel {
  friend class UpstreamConnectionNode;
  friend class UpstreamManager;

 protected:
  ref_<StrandStorageBucket> _storage;
  void destroy_impl() override;
  void initialize() override;

 public:
  UpstreamRootNode(const LinkStrandRef &strand);
};

class UpstreamConnectionNode : public NodeModel {
  friend class UpstreamRootNode;

  ref_<Client> _client;
  ref_<UpstreamRootNode> _parent;
  ref_<RemoteRootNode> _responder_node;

  bool _enabled = true;
  string_ _connection_name;
  string_ _url;
  string_ _token;
  string_ _role;

  ref_<ValueNodeModel> _enabled_node;
  ref_<ValueNodeModel> _url_node;
  ref_<ValueNodeModel> _token_node;
  ref_<ValueNodeModel> _role_node;

  ref_<NodeModel> _status_node;
  ref_<NodeModel> _remote_id_node;
  ref_<NodeModel> _remote_path_node;
  // todo add responder_path

 public:
  UpstreamConnectionNode(const LinkStrandRef &strand,
                         ref_<UpstreamRootNode> &&parent,
                         ref_<NodeModel> &&profile);
  ~UpstreamConnectionNode();

  void update_node_values();

 protected:
  void destroy_impl() override;

  void save_upstream() const;
  void save_extra(VarMap &map) const override;
  void load_extra(VarMap &map) override;

  void connection_changed();
};

}  // namespace dsa

#endif  // DSA_SDK_UPSTREAM_NODES_H
