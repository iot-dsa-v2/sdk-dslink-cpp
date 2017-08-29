#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <boost/asio/deadline_timer.hpp>
#include <functional>
#include <utility>

#include "core/link_strand.h"
#include "crypto/handshake_context.h"
#include "message/base_message.h"
#include "util/util.h"

namespace dsa {
class App;
class Session;
class Server;
class Client;

class Message;

typedef std::function<void(const boost::system::error_code &error)>
    WriteHandler;

class Connection : public SharedClosable<Connection> {
  friend class Session;

 public:
  enum : size_t {
    DEFAULT_BUFFER_SIZE = 1024,
    MAX_BUFFER_SIZE = 65536

  };

 public:
  virtual std::string name() = 0;

  enum Protocol { TCP };

  enum {
    PublicKeyLength = 65,
    SaltLength = 32,
    AuthLength = 32,
    MinF0Length = StaticHeaders::TotalSize + 2 +  // client dsa version
                  1 +                             // client dsid length
                  1 +                             // client dsid content
                  PublicKeyLength +               // client public key
                  1 +                             // client security preference
                  SaltLength,                     // client salt
    MinF1Length = StaticHeaders::TotalSize + 1 +  // broker dsid length
                  1 +                             // broker dsid content
                  PublicKeyLength +               // broker public key
                  SaltLength,                     // broker salt
    MinF2Length = StaticHeaders::TotalSize + 2 +  // client token length
                  0 +                             // client token content
                  1 +                             // client is requester
                  1 +                             // client is responder
                  2 +                             // client session id length
                  0 +                             // client session id
                  AuthLength,                     // client auth
    MinF3Length = StaticHeaders::TotalSize + 2 +  // session id length
                  1 +                             // session id content
                  2 +                             // client path length
                  1 +                             // client path content
                  AuthLength,                     // broker auth
  };

 protected:
  size_t _preferred_buffer_size = DEFAULT_BUFFER_SIZE;
  size_t _max_buffer_size = MAX_BUFFER_SIZE;

 public:
  size_t preferred_buffer_size() const { return _preferred_buffer_size; };
  size_t max_buffer_size() const { return _max_buffer_size; };

  virtual void write(const uint8_t *data, size_t size,
                     WriteHandler &&callback) = 0;

  static void close_in_strand(shared_ptr_<Connection> &&connection);

  std::function<void(MessagePtr)> on_read_message;
  std::function<void()> on_read_message_error;

  // as client
  virtual void connect();
  // as server
  virtual void accept();

  virtual void start() noexcept = 0;
  const std::string &dsid() { return _handshake_context.dsid(); }

  void set_session(const intrusive_ptr_<Session> &session);

 protected:
  Connection(LinkStrandPtr &strand, uint32_t handshake_timeout_ms,
             const std::string &dsid_prefix, const std::string &path = "");
  virtual ~Connection() = default;

  HandshakeContext _handshake_context;
  LinkStrandPtr _strand;

  // this should rarely be touched
  intrusive_ptr_<Session> _session;

  std::vector<uint8_t> _read_buffer;
  std::vector<uint8_t> _write_buffer;

  void close_impl() override;

  std::vector<uint8_t> _shared_secret;
  std::vector<uint8_t> _other_public_key;
  std::vector<uint8_t> _other_salt;
  std::vector<uint8_t> _auth;
  std::vector<uint8_t> _auth_check;
  std::vector<uint8_t> _other_auth;
  std::string _session_id;
  std::string _other_dsid;
  std::string _path;
  std::string _token;
  std::string _other_token;
  std::string _previous_session_id;

  bool _is_requester;
  bool _is_responder;
  bool _security_preference;
  uint32_t _pending_messages{0};
  boost::asio::deadline_timer _deadline;
  uint32_t _handshake_timeout_ms{1000};

  //  virtual void read_loop(size_t from_prev, const boost::system::error_code
  //  &error, size_t bytes_transferred) = 0;

  static bool valid_handshake_header(StaticHeaders &header,
                                     size_t expected_size,
                                     MessageType expected_type);

  void success_or_close(const boost::system::error_code &error);

  void timeout(const boost::system::error_code &error);

  void reset_standard_deadline_timer();

  void post_message(MessagePtr &&msg);

  // server connection
 protected:
  void on_server_connect() throw(const std::runtime_error &);

  void on_receive_f0(MessagePtr &&msg);
  void on_receive_f2(MessagePtr &&msg);

  // handshake functions
  bool parse_f0(size_t size);
  bool parse_f2(size_t size);
  size_t load_f1(std::vector<uint8_t> &buf);
  size_t load_f3(std::vector<uint8_t> &buf);

  // client connection
 protected:
  std::string _client_token;

  void on_client_connect() throw(const std::runtime_error &);

  void start_client_f0();
  void on_receive_f1(MessagePtr &&msg);
  void on_receive_f3(MessagePtr &&msg);

  // handshake functions
  bool parse_f1(size_t size);
  bool parse_f3(size_t size);
  size_t load_f0(std::vector<uint8_t> &buf);
  size_t load_f2(std::vector<uint8_t> &buf);
};

}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
