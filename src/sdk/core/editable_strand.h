#ifndef DSA_SDK_CONFIG_H
#define DSA_SDK_CONFIG_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "link_strand.h"
#include "shared_strand_ref.h"

#include <boost/asio/io_context.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace dsa {

class NodeModelBase;
class Connection;
class App;

typedef ref_<NodeModelBase> ModelRef;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class EditableStrand : public LinkStrand {
 protected:
  std::unique_ptr<ECDH> _ecdh;
  // modules
  ref_<ClientManager> _client_manager;
  ref_<Authorizer> _authorizer;
  ref_<OutgoingStreamAcceptor> _stream_acceptor;
  ref_<SessionManager> _session_manager;
  ref_<Storage> _storage;

  // inject a function and run it as soon as possible
  // the callback must retain a direct or indirect ref to the strand
  void inject(std::function<void()>&&) override;

  std::mutex _inject_mutex;
  std::vector<std::function<void()>> _inject_queue;
  std::function<void()> _inject_callback;
  void _prepare_inject_callback();

 public:
  // simple version for testing purpose
  static ref_<EditableStrand> make_default(shared_ptr_<App> app);

  explicit EditableStrand(boost::asio::io_context::strand* strand,
                          std::unique_ptr<ECDH>&& ecdh);
  ~EditableStrand() override;

  void set_client_manager(ref_<ClientManager> p);
  void set_authorizer(ref_<Authorizer> p);
  void set_stream_acceptor(ref_<OutgoingStreamAcceptor> p);
  void set_session_manager(ref_<SessionManager> p);
  void set_storage(ref_<Storage> p);

  void set_responder_model(ModelRef&& root_model, size_t timer_interval = 60);
  bool is_responder_set() { return _stream_acceptor != nullptr; }

  void destroy_impl() override;

  void check_injected() override;
};

typedef std::function<shared_ptr_<Connection>(const SharedLinkStrandRef& shared_strand)>
    ClientConnectionMaker;

class WrapperStrand : public DestroyableRef<WrapperStrand> {
 public:
  virtual ~WrapperStrand() = default;

  ref_<EditableStrand> strand;
  string_ dsid_prefix;

  // server features
  string_ server_host{"0.0.0.0"};
  uint16_t tcp_server_port{0};
  int32_t tcp_secure_port{-1};

  // client features
  bool secure = false;

  string_ tcp_host;
  int32_t tcp_port{0};

  string_ ws_host;
  uint16_t ws_port{0};
  string_ ws_path;

  string_ client_token;

  ClientConnectionMaker client_connection_maker;

  string_ get_dsid() const;

 protected:
  void destroy_impl() override {
    if (strand != nullptr) {
      auto p_strand = strand.get();
      p_strand->dispatch([strand = std::move(strand)]() { strand->destroy(); });
      client_connection_maker = nullptr;
    }
  }
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H
