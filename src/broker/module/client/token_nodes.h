#ifndef DSA_SDK_TOKEN_NODES_H
#define DSA_SDK_TOKEN_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class BrokerClientManager;
class StrandStorageBucket;
class TokenNode;
class ValueNodeModel;

class TokensRoot : public NodeModel {
  friend class BrokerClientManager;
  friend class TokenNode;

 protected:
  ref_<StrandStorageBucket> _storage;

 public:
  TokensRoot(const LinkStrandRef &strand);
  ~TokensRoot() override;
};

class TokenNode : public NodeModel {
  friend class BrokerClientManager;
  friend class TokensRoot;

 protected:
  ref_<TokensRoot> _parent;

  string_ _token;
  string_ _role;

  string_ _time_range;
  // valid time, ms since epoch
  int64_t _valid_from = LLONG_MIN;
  int64_t _valid_to = LLONG_MIN;

  // how many times this tokne can be used
  int64_t _count = -1;
  // how many session is allowed
  int64_t _max_session = 1;
  // when token is removed, should all link be removed with it
  bool _managed = false;

  ref_<NodeModel> _token_node;
  ref_<ValueNodeModel> _role_node;
  ref_<ValueNodeModel> _time_range_node;
  ref_<ValueNodeModel> _count_node;
  ref_<ValueNodeModel> _max_session_node;
  ref_<ValueNodeModel> _managed_node;

  void udpate_timerange();

 public:
  TokenNode(const LinkStrandRef &strand, ref_<TokensRoot> &&parent,
            ref_<NodeModel> &&profile, const string_ &token = "",
            const string_ &role = "", const string_ &time_range = "",
            int64_t count = -1, int64_t max_session = 1, bool managed = false);
  ~TokenNode() override;
  // token has count and not expired
  bool is_valid();
  void remove_all_clients();
  void regenerate();
  void save_token() const;
};
}  // namespace dsa

#endif  // DSA_SDK_TOKEN_NODES_H
