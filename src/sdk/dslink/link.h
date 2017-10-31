#ifndef DSA_SDK_CONFIG_LOADER_H
#define DSA_SDK_CONFIG_LOADER_H

#include "core/config.h"

namespace dsa {
class App;
class TcpServer;
class Client;

class DsLink : public WrapperConfig {
 public:
  DsLink(int argc, const char *argv[], const string_ &link_name,
         const string_ &version);
  ~DsLink();
  App &get_app() const { return *_app; }

 private:
  std::unique_ptr<App> _app;
  std::unique_ptr<TcpServer> _tcp_server;
  std::unique_ptr<Client> _tcp_client;

  std::function<void()> _on_ready;
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
    strand->set_responder_model(make_ref_<NodeClass>(strand));
  }

  void run(std::function<void()> &&on_ready = nullptr);
};
}

#endif  // DSA_SDK_CONFIG_LOADER_H
