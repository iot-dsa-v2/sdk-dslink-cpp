#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <memory>
#include <functional>

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "util/util.h"
#include "message/static_header.h"
#include "app.h"

typedef boost::function<void()> WriteCallback;
typedef boost::function<void(dsa::Buffer::MessageBuffer)> ReadCallback;

namespace dsa {

/**
 * handshake logic
 * split and join binary data into message frame
 */
class Connection : public InheritableEnableShared<Connection> {
 protected:
  Connection(const App &app);
  const App &_app;

  BufferPtr _read_buffer;
  BufferPtr _write_buffer;
  BufferPtr _shared_secret;
  BufferPtr _other_public_key;
  BufferPtr _other_salt;
  BufferPtr _other_dsid;
  BufferPtr _other_auth;
  BufferPtr _other_token;
  BufferPtr _session_id;
  BufferPtr _path;
  BufferPtr _token;
  BufferPtr _auth;
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

  // for this to be successful, _other_salt and _other_public_key need to valid
  void compute_secret();

  enum {
    STATIC_HEADER_LENGTH = 15,
    PUBLIC_KEY_LENGTH = 65,
    SALT_LENGTH = 32,
    AUTH_LENGTH = 32,
    MIN_F0_LENGTH = STATIC_HEADER_LENGTH +
        2 +                 // client dsa version
        1 +                 // client dsid length
        1 +                 // client dsid content
        PUBLIC_KEY_LENGTH + // client public key
        1 +                 // client security preference
        SALT_LENGTH,        // client salt
    MIN_F1_LENGTH = STATIC_HEADER_LENGTH +
        1 +                 // broker dsid length
        1 +                 // broker dsid content
        PUBLIC_KEY_LENGTH + // broker public key
        SALT_LENGTH,        // broker salt
    MIN_F2_LENGTH = STATIC_HEADER_LENGTH +
        2 +                 // client token length
        0 +                 // client token content
        1 +                 // client is requester
        1 +                 // client is responder
        AUTH_LENGTH,        // client auth
    MIN_F3_LENGTH = STATIC_HEADER_LENGTH +
        2 +                 // session id length
        1 +                 // session id content
        2 +                 // client path length
        1 +                 // client path content
        AUTH_LENGTH,        // broker auth
  };

  ReadCallback read_handler;

  static bool valid_handshake_header(StaticHeaders &header, size_t expected_size, uint8_t expected_type);

  void success_or_close(const boost::system::error_code &error);

 public:
  void set_read_handler(ReadCallback callback);
  virtual void destroy();
  virtual void write(BufferPtr buf, size_t size, WriteCallback callback) = 0;
  virtual void close() = 0;
  virtual void start() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
