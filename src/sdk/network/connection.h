#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/mutex.hpp>
#include <functional>
#include <utility>

#include "core/link_strand.h"
#include "core/session.h"
#include "crypto/handshake_context.h"
#include "message/enums.h"

#include "util/enable_shared.h"

namespace dsa {
class App;
class Session;
class Server;
class Client;

class StaticHeaders;

class Message;
typedef ref_<Message> MessageRef;

typedef std::function<void(const boost::system::error_code &error)>
    WriteHandler;

class ConnectionWriteBuffer {
 public:
//<<<<<<< HEAD
//  static const size_t DEFAULT_BUFFER_SIZE = 8192;
//  // write buffer will have 1/16 unusable part by default
//  // which seems to improve the performance on windows
//  static const size_t MAX_BUFFER_SIZE = 8192 * 15;
//
//=======
  virtual size_t max_next_size() const = 0;
  virtual void add(const Message &msg, int32_t rid, int32_t ack_id) = 0;
  virtual void write(WriteHandler &&callback) = 0;
};

class Connection : public SharedClosable<Connection> {
  friend class Session;

 public:

  virtual std::string name() = 0;

 public:
  void dispatch_in_strand(std::function<void()> &&callback) override {
    return _strand->dispatch(std::move(callback));
  }

  std::function<bool(MessageRef)> on_read_message;

  //  virtual void write(const uint8_t *data, size_t size,
  //                     WriteHandler &&callback) = 0;

  virtual std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() = 0;

  boost::mutex read_loop_mutex;

  // as client
  virtual void connect();
  // as server
  virtual void accept();

  const std::string &dsid() { return _handshake_context.dsid(); }

  void set_session(const ref_<Session> &session);

  Session *session() { return _session.get(); }

 protected:
  Connection(LinkStrandRef &strand, uint32_t handshake_timeout_ms,
             const std::string &dsid_prefix, const std::string &path = "");
  virtual ~Connection() = default;

  HandshakeContext _handshake_context;
  LinkStrandRef _strand;

  // this should rarely be touched
  ref_<Session> _session;


  void close_impl() override;

  std::string _path;

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

  std::vector<MessageRef> _batch_post;
  bool post_message(MessageRef &&msg);

  // server connection
 protected:
  //  void on_server_connect() throw(const std::runtime_error &);

  bool on_receive_f0(MessageRef &&msg);
  bool on_receive_f2(MessageRef &&msg);

  // client connection
 protected:
  std::string _client_token;

  static void on_client_connect(shared_ptr_<Connection> connection) throw(
      const std::runtime_error &);

  void start_client_f0();
  bool on_receive_f1(MessageRef &&msg);
  bool on_receive_f3(MessageRef &&msg);
};

}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
