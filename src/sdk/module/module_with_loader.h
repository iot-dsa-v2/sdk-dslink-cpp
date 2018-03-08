#ifndef DSA_SDK_MODULE_LOADER_H
#define DSA_SDK_MODULE_LOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "module.h"

#include "boost/filesystem.hpp"
namespace bf = boost::filesystem;

namespace dsa {

class ModuleWithLoader : public Module {
 private:
  // Default one that we will get remaining ones after file load
  ref_<Module> _default_module;

  // The loaded modules from file
  std::vector<boost::shared_ptr<Module>> _modules;

 protected:
  ref_<Storage> create_storage(App& app, ref_<LinkStrand>& strand) override;
  shared_ptr_<Logger> create_logger(App& app,
                                    ref_<LinkStrand>& strand) override;
  ref_<ClientManager> create_client_manager(App& app,
                                            ref_<LinkStrand>& strand) override;
  ref_<Authorizer> create_authorizer(App& app,
                                     ref_<LinkStrand>& strand) override;
  shared_ptr_<LoginManager> create_login_manager(
      App& app, ref_<LinkStrand>& strand) override;

 public:
  ModuleWithLoader(bf::path lib_path, ref_<Module>&& default_module);

  void add_module_node(NodeModel& module_node, BrokerPubRoot& pub_root) override;
  void add_web_handler() override;
};
}

#endif  // DSA_SDK_MODULE_LOADER_H
