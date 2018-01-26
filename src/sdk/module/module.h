#ifndef DSA_SDK_MODULE_H
#define DSA_SDK_MODULE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"

namespace dsa {

class Module : public DestroyableRef<Module> {
 public:
  virtual void init() = 0;
  virtual ref_<ClientManager> get_client_manager() = 0;
  virtual ref_<Authorizer> get_authorizer() = 0;

  virtual ref_<SessionManager> get_session_manager() = 0;
  virtual ref_<Logger> get_logger() = 0;
  virtual ref_<Storage> get_storage() = 0;

  virtual void add_module_node() = 0;
  virtual void add_web_handler() = 0;
};
}

#endif  // DSA_SDK_MODULE_H
