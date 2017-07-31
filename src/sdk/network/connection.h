#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <functional>
#include <atomic>
#include <utility>

#include <boost/asio/deadline_timer.hpp>

#include "util/util.h"
#include "message/static_header.h"
#include "security_context.h"

namespace boost {
namespace system {
class error_code;
}
}

namespace dsa {
class App;
class Session;

typedef std::function<void()> WriteHandler;
typedef std::function<void(const std::shared_ptr<Session> &, Buffer::SharedBuffer)> MessageHandler;

class Connection : public InheritableEnableShared<Connection> {
 public:
  class Config {
   private:
    std::string _host{"127.0.0.1"};
    unsigned short _port{8080};
    BufferPtr _token{std::make_shared<Buffer>("")};
    BufferPtr _session_id{std::make_shared<Buffer>("")};
    // handshake timeout is in milliseconds
    unsigned int _handshake_timeout{1000};
    unsigned int _max_pending_messages{20};
    MessageHandler _message_handler{[](std::shared_ptr<Session> s, Buffer::SharedBuffer b){}};

   public:
    Config(std::string host, unsigned short port) : _host(std::move(host)), _port(port) {}
    Config(MessageHandler message_handler) : _message_handler(std::move(message_handler)) {}
    Config() = default;
    Config(const Config &) = default;
    Config &operator=(const Config &) = default;
    Config(Config &&) = default;
    Config &operator=(Config &&) = default;
    ~Config() = default;

    void set_host(std::string host) { _host = std::move(host); }
    void set_port(unsigned short port) { _port = port; }
    void set_token(const std::string &token) { _token = std::make_shared<Buffer>(token); }
    void set_session_id(const std::string &session_id) { _session_id = std::make_shared<Buffer>(session_id); }
    // handshake timeout is in milliseconds
    void set_handshake_timeout(unsigned int timeout) { _handshake_timeout = timeout; }
    void set_max_pending_messages(unsigned int max_pending) { _max_pending_messages = max_pending; }
    void set_message_handler(MessageHandler message_handler) { _message_handler = std::move(message_handler); }

    const std::string &host() const { return _host; }
    unsigned short port() const { return _port; }
    const BufferPtr &token() const { return _token; }
    const BufferPtr &session_id() const { return _session_id; }
    unsigned int handshake_timout() const { return _handshake_timeout; }
    unsigned int max_pending_messages() const { return _max_pending_messages; }
    MessageHandler message_handler() const { return _message_handler; }
  };

  enum Protocol {
    TCP
  };

  enum {
    PublicKeyLength = 65,
    SaltLength = 32,
    AuthLength = 32,
    MinF0Length = StaticHeaders::TotalSize +
        2 +                 // client dsa version
        1 +                 // client dsid length
        1 +                 // client dsid content
        PublicKeyLength +   // client public key
        1 +                 // client security preference
        SaltLength,         // client salt
    MinF1Length = StaticHeaders::TotalSize +
        1 +                 // broker dsid length
        1 +                 // broker dsid content
        PublicKeyLength +   // broker public key
        SaltLength,         // broker salt
    MinF2Length = StaticHeaders::TotalSize +
        2 +                 // client token length
        0 +                 // client token content
        1 +                 // client is requester
        1 +                 // client is responder
        2 +                 // client session id length
        0 +                 // client session id
        AuthLength,         // client auth
    MinF3Length = StaticHeaders::TotalSize +
        2 +                 // session id length
        1 +                 // session id content
        2 +                 // client path length
        1 +                 // client path content
        AuthLength,         // broker auth
  };

  void set_message_handler(MessageHandler handler) { _message_handler = std::move(handler); }
//  void destroy() override;
  virtual void write(BufferPtr buf, size_t size, WriteHandler callback) = 0;
  virtual void close() = 0;
  virtual void connect() = 0;
  virtual void start() throw() = 0;

 protected:
  explicit Connection(const App &app, const Config &config);
  SecurityContext &_security_context;
  Config _config;

  // this should rarely be touched
  std::shared_ptr<Session> _session;

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
  std::atomic_uint _pending_messages{0};
  boost::asio::deadline_timer _deadline;
  MessageHandler _message_handler;

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

  // for this to be successful, _other_salt and _other_public_key need to valid
  void compute_secret();

  static bool valid_handshake_header(StaticHeaders &header, size_t expected_size, uint8_t expected_type);

  void success_or_close(const boost::system::error_code &error);

  void timeout(const boost::system::error_code &error);

  void reset_standard_deadline_timer();
};

typedef std::shared_ptr<Connection> ConnectionPtr;

}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
