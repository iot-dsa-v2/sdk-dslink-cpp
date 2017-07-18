#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <queue>

#include <boost/function.hpp>
#include <boost/asio.hpp>

#include "util/util.h"
#include "message/static_header.h"

typedef boost::function0<void> WriteCallback;
typedef boost::function1<void, dsa::Buffer::MessageBuffer> ReadCallback;

namespace dsa {

/**
 * handshake logic
 * split and join binary data into message frame
 */

class Connection : public EnableShared<Connection> {
 protected:
  Connection(boost::asio::io_service &io_service1);
  boost::asio::io_service &io_service;

  BufferPtr _buffer;
  BufferPtr _shared_secret;
  BufferPtr _other_public_key;
  BufferPtr _other_salt;
  BufferPtr _other_dsid;
  BufferPtr _other_auth;
  BufferPtr _other_token;
  BufferPtr _session_id;
  BufferPtr _path;
  uint8_t _dsa_version_major;
  uint8_t _dsa_version_minor;
  bool _is_requester;
  bool _is_responder;
  bool _security_preference;

  // parse handshake messages
  bool parse_f0(size_t size);
  bool parse_f1(size_t size);
  bool parse_f2(size_t size);
  bool parse_f3(size_t size);

  // load handshake messages
  size_t load_f0(Buffer &buf);
  size_t load_f1(Buffer &buf);
  size_t load_f2(Buffer &buf);
  size_t load_f3(Buffer &buf);

  virtual void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) = 0;

  void handle_read(Buffer::MessageBuffer buf);
  
  enum {
    static_header_length = 15,
    public_key_length = 65,
    salt_length = 32,
    auth_length = 32,
    min_f0_length = static_header_length +
        2 +                 // client dsid version
        1 +                 // client dsid length
        20 +                // client dsid content
        public_key_length + // client public key
        1 +                 // client security preference
        salt_length,        // client salt
    min_f1_length = static_header_length +
        1 +                 // broker dsid length
        20 +                // broker dsid content
        public_key_length + // broker public key
        salt_length,        // broker salt
    min_f2_length = static_header_length +
        2 +                 // client token length
        0 +                 // client token content
        1 +                 // client is requester
        1 +                 // client is responder
        auth_length,        // client auth
    min_f3_length = static_header_length +
        2 +                 // session id length
        1 +                 // session id content
        2 +                 // client path length
        1 +                 // client path content
        auth_length,        // broker auth
  };

  ReadCallback read_handler;

  static bool valid_handshake_header(StaticHeader &header, size_t size, uint8_t type);

 public:
  void set_read_handler(ReadCallback callback);
  virtual void write(BufferPtr buf, size_t size, WriteCallback callback) = 0;
  virtual void close() = 0;
  virtual void start() = 0;
  virtual void destroy() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
