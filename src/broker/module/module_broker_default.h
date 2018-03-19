#ifndef DSA_SDK_MODULE_BROKER_DEFAULT_H
#define DSA_SDK_MODULE_BROKER_DEFAULT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "module/module.h"

namespace dsa {

class ModuleBrokerDefault : public Module {
 protected:
  ref_<Storage> create_storage(App& app, const LinkStrandRef &strand) override;
  shared_ptr_<Logger> create_logger(App& app,
                                    const LinkStrandRef &strand) override;
  ref_<ClientManager> create_client_manager(App& app,
                                            const LinkStrandRef &strand) override;
  ref_<Authorizer> create_authorizer(App& app,
                                     const LinkStrandRef &strand) override;
  shared_ptr_<LoginManager> create_login_manager(
      App& app, const LinkStrandRef &strand) override;

 public:
  void add_module_node(NodeModel& module_node, BrokerPubRoot& pub_root) override;
};
}

#endif  // DSA_SDK_MODULE_BROKER_DEFAULT_H
