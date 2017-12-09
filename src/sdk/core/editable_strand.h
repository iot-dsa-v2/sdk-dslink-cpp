#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "link_strand.h"

#include <boost/asio/io_service.hpp>
#include <memory>

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
  ref_<SecurityManager> _security_manager;
  ref_<OutgoingStreamAcceptor> _stream_acceptor;
  ref_<SessionManager> _session_manager;
  std::unique_ptr<Logger> _logger;

 public:
  // simple version for testing purpose
  static ref_<EditableStrand> make_default(shared_ptr_<App> app);

  explicit EditableStrand(boost::asio::io_service::strand* strand,
                          std::unique_ptr<ECDH>&& ecdh);
  ~EditableStrand() override;

  void set_security_manager(ref_<SecurityManager> p);
  void set_stream_acceptor(ref_<OutgoingStreamAcceptor> p);
  void set_session_manager(ref_<SessionManager> p);
  void set_logger(std::unique_ptr<Logger> p);

  void set_responder_model(ModelRef&& root_model, size_t timer_interval = 60);

  void destroy_impl() override;
};

typedef std::function<shared_ptr_<Connection>(
    LinkStrandRef& strand, const string_& previous_session_id,
    int32_t last_ack_id)>
    ClientConnectionMaker;

class WrapperStrand : public DestroyableRef<WrapperStrand> {
 public:
  virtual ~WrapperStrand() = default;

  ref_<EditableStrand> strand;
  string_ dsid_prefix;

  // server features
  string_ server_host{"0.0.0.0"};
  uint16_t tcp_server_port{0};
  uint16_t tcp_secure_port{0};

  // client features
  bool secure = false;

  string_ tcp_host;
  uint16_t tcp_port{0};

  string_ ws_host;
  uint16_t ws_port{0};
  string_ ws_path;

  string_ client_token;

  ClientConnectionMaker client_connection_maker;

 protected:
  void destroy_impl() override {
    if (strand != nullptr) {
      strand->destroy();
      strand.reset();
    }
  }
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
