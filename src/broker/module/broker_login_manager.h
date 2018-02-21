#ifndef DSA_SDK_BROKER_LOGIN_MANAGER_H
#define DSA_SDK_BROKER_LOGIN_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../sdk/web_server/login_manager.h"
#include "util/client_info.h"
#include "core/link_strand.h"

namespace dsa {

class NodeModel;
class ModuleBrokerDefault;

class BrokerLoginManager final : public LoginManager {
  friend class ModuleBrokerDefault;

  ref_<LinkStrand> _strand;

  ref_<NodeModel> _module_node;

 public:
  explicit BrokerLoginManager(ref_<LinkStrand>& strand);
  ~BrokerLoginManager() override;
  void check_login(const string_& username, const string_& password,
                   ClientInfo::GetClientCallback&& callback) override;
  void get_user(const string_& username,
                ClientInfo::GetClientCallback&& callback) override;
};
}

#endif  // DSA_SDK_BROKER_LOGIN_MANAGER_H
