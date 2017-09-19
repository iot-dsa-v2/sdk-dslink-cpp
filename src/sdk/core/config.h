#ifndef DSA_SDK_CONFIG_H_
#define DSA_SDK_CONFIG_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "link_strand.h"

#include <boost/asio/io_service.hpp>

namespace dsa {

template <typename T, typename... Args>
std::unique_ptr<T> make_unique_(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class LinkConfig : public LinkStrand {
 protected:
  // modules
  std::unique_ptr<SecurityManager> _security_manager;
  std::unique_ptr<OutgoingStreamAcceptor> _stream_acceptor;
  std::unique_ptr<SessionManager> _session_manager;
  std::unique_ptr<Logger> _logger;

 public:
  explicit LinkConfig(boost::asio::io_service::strand* strand, ECDH* ecdh);
  virtual ~LinkConfig();

  void set_security_manager(std::unique_ptr<SecurityManager> p);
  void set_stream_acceptor(std::unique_ptr<OutgoingStreamAcceptor> p);
  void set_session_manager(std::unique_ptr<SessionManager> p);
  void set_logger(std::unique_ptr<Logger> p);
};

class WrapperConfig {
 public:
  LinkStrandRef strand;
  std::string dsid_prefix;
  std::string tcp_host;
  uint16_t tcp_port{0};

  uint32_t handshake_timeout_ms = 5000;

  // client configs
  std::string client_token;
};

}  // namespace dsa

#endif  // DSA_SDK_CONFIG_H_
