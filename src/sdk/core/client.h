#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "config.h"
#include "util/enable_shared.h"
#include "util/enable_ref.h"

namespace dsa {

class Connection;
class Session;

class Client : public SharedDestroyable<Client> {
 protected:
  // for Session/Requester/Responder

  // for ClientConnection
  LinkStrandRef _strand;
  uint32_t handshake_timeout_ms;
  std::string _dsid_prefix;
  std::string _client_token;

  ClientConnectionMaker _client_connection_maker;

  shared_ptr_<Connection> _connection;
  ref_<Session> _session;

  void destroy_impl() override;

 public:
  explicit Client(WrapperConfig &config);

  ~Client();

  void dispatch_in_strand(std::function<void()> &&callback) override {
    return _strand->dispatch(std::move(callback));
  }

  Session &get_session() { return *_session; };

  LinkStrand &get_strand() const { return *_strand; }
  uint32_t get_handshake_timeout_ms() const { return handshake_timeout_ms; }
  const std::string &get_dsid_prefix() const { return _dsid_prefix; }
  const std::string &get_client_token() const { return _client_token; }

  void connect();
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_
