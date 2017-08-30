#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/shared_mutex.hpp>
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
  };

 protected:
  size_t _preferred_buffer_size = DEFAULT_BUFFER_SIZE;
  size_t _max_buffer_size = MAX_BUFFER_SIZE;

 public:
  size_t preferred_buffer_size() const { return _preferred_buffer_size; };
  size_t max_buffer_size() const { return _max_buffer_size; };

  virtual void write(const uint8_t *data, size_t size,
                     WriteHandler &&callback) = 0;

  std::function<void(MessagePtr)> on_read_message;
  std::function<void()> on_read_message_error;
  boost::mutex read_loop_mutex;

  // as client
  virtual void connect();
  // as server
  virtual void accept();

  const std::string &dsid() { return _handshake_context.dsid(); }

  void set_session(const intrusive_ptr_<Session> &session);

  boost::asio::strand * asio_strand(){return (*_strand)();}
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

  std::string _session_id;
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
  //  void on_server_connect() throw(const std::runtime_error &);

  void on_receive_f0(MessagePtr &&msg);
  void on_receive_f2(MessagePtr &&msg);

  // client connection
 protected:
  std::string _client_token;

  static void on_client_connect(shared_ptr_<Connection> connection) throw(const std::runtime_error &);

  void start_client_f0();
  void on_receive_f1(MessagePtr &&msg);
  void on_receive_f3(MessagePtr &&msg);
};

}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
