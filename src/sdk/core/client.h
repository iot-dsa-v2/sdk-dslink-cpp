#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "config.h"
#include "util/enable_ref.h"

namespace dsa {

class Connection;
class Session;

class Client : public DestroyableRef<Client> {
 protected:
  // for Session/Requester/Responder

  // for ClientConnection
  LinkStrandRef _strand;
  string_ _dsid_prefix;
  string_ _client_token;

  ClientConnectionMaker _client_connection_maker;

  shared_ptr_<Connection> _connection;
  ref_<Session> _session;

  void destroy_impl() override;

 public:
  Client(WrapperConfig &config);

  ~Client();

  Session &get_session() { return *_session; };

  LinkStrand &get_strand() const { return *_strand; }
  const string_ &get_dsid_prefix() const { return _dsid_prefix; }
  const string_ &get_client_token() const { return _client_token; }

  void connect();
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_
