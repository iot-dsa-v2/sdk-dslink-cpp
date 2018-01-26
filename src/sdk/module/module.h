#ifndef DSA_SDK_MODULE_H
#define DSA_SDK_MODULE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "util/app.h"

namespace dsa {

class Module : public DestroyableRef<Module> {
 private:
  ref_<Storage> _storage;
  ref_<Logger> _logger;
  ref_<ClientManager> _client_manager;
  ref_<Authorizer> _authorizer;

 protected:
  virtual ref_<Storage> create_storage(App& app, ref_<LinkStrand> strand);
  virtual ref_<Logger> create_logger(App& app, ref_<LinkStrand> strand);
  virtual ref_<ClientManager> create_client_manager(App& app, ref_<LinkStrand> strand);
  virtual ref_<Authorizer> create_authorizer(App& app, ref_<LinkStrand> strand);

 public:
  virtual void init_all(App& app, ref_<LinkStrand> strand);

  void init_storage(App& app, ref_<LinkStrand> strand);
  void init_logger(App& app, ref_<LinkStrand> strand);
  void init_client_manager(App& app, ref_<LinkStrand> strand);
  void init_authorizer(App& app, ref_<LinkStrand> strand);

  ref_<Storage> get_storage();
  ref_<Logger> get_logger();
  ref_<ClientManager> get_client_manager();
  ref_<Authorizer> get_authorizer();

  // Override here for add module node in main node
  virtual void add_module_node();

  // Override here for addinf web handler
  virtual void add_web_handler();

 public:
  Module();

};
}

#endif  // DSA_SDK_MODULE_H
