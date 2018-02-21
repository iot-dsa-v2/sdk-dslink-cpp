#ifndef DSA_BROKER_SECURITY_MANAGER_H
#define DSA_BROKER_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "module/client_manager.h"

namespace dsa {
class ModuleLoader;
class DsBroker;

class BrokerClientManager : public ClientManager {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

 public:
  BrokerClientManager(LinkStrandRef& strand) : _strand(strand){};

  void get_client(const string_& dsid, const string_& auth_token,
                  ClientInfo::GetClientCallback&& callback) override;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H
