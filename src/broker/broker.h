#ifndef DSA_BROKER_H
#define DSA_BROKER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <module/module.h>
#include "core/editable_strand.h"
#include "core/session.h"

namespace dsa {

class App;
class TcpServer;
class WebServer;
class Client;
class BrokerConfig;
class ModuleLoader;

class DsBroker final : public WrapperStrand {
  friend class SubscribeMerger;
  friend class ListMerger;

 private:
  string_ _close_token;

 public:
  DsBroker(ref_<BrokerConfig>&& config, ref_<Module>&&  modules,
           const shared_ptr_<App>& app = nullptr);
  ~DsBroker() final;

  void run(bool wait = true);
  void wait();
  shared_ptr_<App>& get_app() { return _app; }
  const ref_<BrokerConfig>& get_config() const { return _config; };

  string_ get_close_token() { return _close_token; }

  // not from config, coming from tcp_server's active server port
  int32_t get_active_server_port();

 protected:
  bool _own_app = false;
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;
  shared_ptr_<WebServer> _web_server;
  ref_<BrokerConfig> _config;
  // initialization

  void init(ref_<Module>&&  modules);
  void destroy_impl() final;

 public:
};
}

#endif  // DSA_BROKER_H
