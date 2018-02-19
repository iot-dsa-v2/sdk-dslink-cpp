#ifndef DSA_SDK_MODULE_H
#define DSA_SDK_MODULE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "util/app.h"
#include "web_server/login_manager.cc"

namespace dsa {
class NodeModel;

class Module : public DestroyableRef<Module> {
 protected:
  ref_<Storage> _storage;
  shared_ptr_<Logger> _logger;
  ref_<ClientManager> _client_manager;
  ref_<Authorizer> _authorizer;
  shared_ptr_<LoginManager> _login_manager;

 protected:
  virtual ref_<Storage> create_storage(App& app, ref_<LinkStrand> strand);
  virtual shared_ptr_<Logger> create_logger(App& app, ref_<LinkStrand> strand);
  virtual ref_<ClientManager> create_client_manager(App& app,
                                                    ref_<LinkStrand> strand);
  virtual ref_<Authorizer> create_authorizer(App& app, ref_<LinkStrand> strand);
  virtual shared_ptr_<LoginManager> create_login_manager(
      App& app, ref_<LinkStrand> strand);

  void destroy_impl() override;

 public:
  virtual void init_all(App& app, ref_<LinkStrand> strand);

  void init_storage(App& app, ref_<LinkStrand> strand);
  void init_logger(App& app, ref_<LinkStrand> strand);
  void init_client_manager(App& app, ref_<LinkStrand> strand);
  void init_authorizer(App& app, ref_<LinkStrand> strand);
  void init_login_manager(App& app, ref_<LinkStrand> strand);

  ref_<Storage> get_storage();
  shared_ptr_<Logger> get_logger();
  ref_<ClientManager> get_client_manager();
  ref_<Authorizer> get_authorizer();
  shared_ptr_<LoginManager> get_login_manager();

  // Override here for add module node in main node
  virtual void add_module_node(ref_<NodeModel>& module_node);

  // Override here for addinf web handler
  virtual void add_web_handler();

 public:
  Module();
  virtual ~Module() = default;
};
}

#endif  // DSA_SDK_MODULE_H
