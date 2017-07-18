#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <queue>

#include <boost/function.hpp>
#include <boost/asio.hpp>

#include "../util/util.h"

typedef boost::function0<void> WriteCallback;
typedef boost::function1<void, dsa::Buffer::MessageBuffer> ReadCallback;

namespace dsa {

/**
 * handshake logic
 * split and join binary data into message frame
 */

class Connection : public EnableShared<Connection> {
 protected:
  enum { static_header_size = 11 };

  Connection(boost::asio::io_service &io_service1);
  boost::asio::io_service &io_service;

  BufferPtr _buffer;
  BufferPtr _shared_secret;
  BufferPtr _other_public_key;
  BufferPtr _other_salt;
  BufferPtr _other_dsid;
  BufferPtr _other_auth;
  BufferPtr _other_token;
  uint8_t _dsa_version_major;
  uint8_t _dsa_version_minor;
  std::string path;

  // parse handshake messages
  bool parse_f0(size_t bytes_transferred);
  bool parse_f1(size_t bytes_transferred);
  bool parse_f2(size_t bytes_transferred);
  bool parse_f3(size_t bytes_transferred);

  // load handshake messages
  size_t load_f0(Buffer &buf);
  size_t load_f1(Buffer &buf);
  size_t load_f2(Buffer &buf);
  size_t load_f3(Buffer &buf);

  virtual void read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) = 0;

  void handle_read(Buffer::MessageBuffer buf);

 private:
  enum {
    min_f0_length = 11 + // static header
        2 +  // client dsid version
        1 +  // client dsid length
        20 + // client dsid content
        65 + // client public key
        1 +  // client security preference
        32,  // client salt
    min_f1_length = 11 + // static header
        1 +  // broker dsid length
        20 + // broker dsid content
        65 + // broker public key
        32,  // broker salt
    min_f2_length = 11 + // static header
        2 +  // client token length
        0 +  // client token content
        1 +  // client is requester
        1 +  // client is responder
        32,  // client auth
    min_f3_length = 11 + // static header
        2 +  // session id length
        1 +  // session id content
        2 +  // client path length
        1 +  // client path content
        32,  // broker auth
    public_key_length = 65,
    salt_length = 32,
    auth_length = 32
  };

  ReadCallback read_handler;

 public:
  void set_read_handler(ReadCallback callback);
  virtual void write(BufferPtr buf, size_t size, WriteCallback callback) = 0;
  virtual void close() = 0;
  virtual void start() = 0;
  virtual void destroy() = 0;
};
}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
