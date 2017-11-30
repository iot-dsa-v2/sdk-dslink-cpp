#ifndef DSA_BROKER_H
#define DSA_BROKER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/editable_strand.h"
#include "core/session.h"

namespace dsa {

class App;
class TcpServer;
class Client;
class BrokerConfig;
class ModuleLoader;

class DsBroker final : public WrapperStrand {
  friend class SubscribeMerger;
  friend class ListMerger;

 private:
 public:
  DsBroker(ref_<BrokerConfig>&& config, ModuleLoader& modules,
           const shared_ptr_<App>& app = nullptr);
  ~DsBroker();

  void run();
  shared_ptr_<App>& get_app() { return _app; }
  const ref_<BrokerConfig>& get_config() const { return _config; };

 protected:
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;
  ref_<BrokerConfig> _config;
  // initialization

  void init(ModuleLoader& modules);
  void destroy_impl() final;
 public:
};
}

#endif  // DSA_BROKER_H
