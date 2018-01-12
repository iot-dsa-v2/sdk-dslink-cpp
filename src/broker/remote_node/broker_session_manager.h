#ifndef DSA_BROKER_SESSION_MANAGER_H
#define DSA_BROKER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../remote_node/broker_client.h"
#include "module/session_manager.h"

namespace dsa {

class DownstreamRoot;

class BrokerSessionManager final : public SessionManager {
  friend class BrokerClient;

  std::unordered_map<string_, ref_<BrokerClient>> _clients;

  LinkStrandRef _strand;

  ref_<DownstreamRoot> _downstream_root;

  void client_destroyed(BrokerClient &client);

 protected:
  void destroy_impl() final;

 public:
  BrokerSessionManager(LinkStrandRef strand,
                       ref_<DownstreamRoot> downstream_root);
  ~BrokerSessionManager() final;
  void get_session(const string_ &dsid, const string_ &auth_token,
                   int32_t last_token,
                   Session::GetSessionCallback &&callback) final;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H
