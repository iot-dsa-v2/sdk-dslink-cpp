#ifndef DSA_SDK_CONFIG_LOADER_H
#define DSA_SDK_CONFIG_LOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/config.h"
#include "core/session.h"
#include "subscribe_merger.h"
#include "list_merger.h"

namespace dsa {
class App;
class TcpServer;
class Client;

class DsLink : public WrapperConfig {
 public:
  DsLink(int argc, const char *argv[], const string_ &link_name,
         const string_ &version);
  ~DsLink();
  App &get_app();

 private:
  shared_ptr_<App> _app;
  shared_ptr_<TcpServer> _tcp_server;
  ref_<Client> _tcp_client;

  std::unordered_map<std::string, ref_<SubscribeMerger>> subscribe_mergers;
  std::unordered_map<std::string, ref_<ListMerger>> list_mergers;

  bool _running = false;

  // initialization
  std::unique_ptr<ECDH> load_private_key();
  void parse_thread(size_t thread);
  void parse_url(const string_ &url);
  void parse_log(const string_ &log, LinkConfig &config);
  void parse_name(const string_ &name);
  void parse_server_port(uint16_t port);

 public:
  void init_responder(ref_<NodeModelBase> &&root_node);
  template <class NodeClass>
  void init_responder() {
    init_responder(make_ref_<NodeClass>(strand));
  }

  void run(Session::OnConnectedCallback &&on_ready = nullptr,
           uint8_t callback_type = Session::FIRST_CONNECTION);

  // requester functions
  

  ref_<IncomingInvokeStream> invoke(const string_ &path,
                                    IncomingInvokeStreamCallback &&callback,
                                    ref_<const InvokeRequestMessage> &&message);

  ref_<IncomingSetStream> set(const string_ &path,
                              IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message);
};
}

#endif  // DSA_SDK_CONFIG_LOADER_H
