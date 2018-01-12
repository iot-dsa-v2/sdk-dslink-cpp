#ifndef DSA_SDK_BASE_SOCKET_CONNECTION_H_
#define DSA_SDK_BASE_SOCKET_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>

#include "connection.h"
#include "util/enable_shared.h"

namespace dsa {

typedef boost::asio::ip::tcp::socket tcp_socket;

// Base TCP socket connection. Used for DSA connections over TCP.
// Handles DSA handshake, combining outgoing messages,
// and separating incoming messages.
class BaseSocketConnection : public Connection {
 protected:
  // write buffer will have 2/32 unusable part by default
  // which seems to improve the performance
  static const size_t DEFAULT_BUFFER_SIZE = 8196;
  static const size_t MAX_BUFFER_SIZE = DEFAULT_BUFFER_SIZE * 15;

  // current pos of reading data
  size_t _read_current = 0;
  // next pos to start read (end of partial reading data)
  size_t _read_next = 0;
  std::vector<uint8_t> _read_buffer;

  std::vector<uint8_t> _write_buffer;

  void read_loop_(shared_ptr_<Connection> &&connection, size_t from_prev,
                  const boost::system::error_code &error,
                  size_t bytes_transferred);

  std::atomic_bool _socket_open{true};
  void on_deadline_timer_(const boost::system::error_code &error,
                          shared_ptr_<Connection> &&sthis);

 public:
  BaseSocketConnection(LinkStrandRef &strand, const string_ &dsid_prefix,
                       const string_ &path = "");

  virtual void start_read(shared_ptr_<Connection> &&connection) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_BASE_SOCKET_CONNECTION_H_
