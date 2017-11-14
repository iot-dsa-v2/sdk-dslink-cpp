#ifndef DSA_BROKER_H
#define DSA_BROKER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/config.h"
#include "core/session.h"

namespace dsa {

class App;
class TcpServer;
class Client;

class DsBroker final : public WrapperConfig, public DestroyableRef<DsBroker> {
  friend class SubscribeMerger;
  friend class ListMerger;
private:


 public:


 protected:
  void destroy_impl() final;

 private:
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;

  // initialization


 public:
};
}

#endif  // DSA_BROKER_H
