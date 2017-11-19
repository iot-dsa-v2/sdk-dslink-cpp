#ifndef DSA_SDK_CONNECTION_H_
#define DSA_SDK_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/deadline_timer.hpp>
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

struct StaticHeaders;

class Message;
typedef ref_<Message> MessageRef;

typedef std::function<void(const boost::system::error_code &error)>
    WriteHandler;

class ConnectionWriteBuffer {
 public:
  virtual size_t max_next_size() const = 0;
  virtual void add(const Message &msg, int32_t rid, int32_t ack_id) = 0;
  virtual void write(WriteHandler &&callback) = 0;
  virtual ~ConnectionWriteBuffer()=default;
};

class Connection : public SharedDestroyable<Connection> {
  friend class Session;
  friend class Client;

 public:
  virtual string_ name() = 0;

 public:
  void post_in_strand(std::function<void()> &&callback) override {
    return _strand->post(std::move(callback));
  }

  std::function<bool(MessageRef)> on_read_message;

  //  virtual void write(const uint8_t *data, size_t size,
  //                     WriteHandler &&callback) = 0;

  virtual std::unique_ptr<ConnectionWriteBuffer> get_write_buffer() = 0;

  // as client
  virtual void connect(size_t reconnect_interval);
  // as server
  virtual void accept();

  const string_ &get_dsid() { return _handshake_context.dsid(); }
  const string_ &get_remote_dsid() { return _handshake_context.remote_dsid(); }
  const string_ &get_remote_path() { return _remote_path; }

  void set_session(const ref_<Session> &session);

  Session *session() { return _session.get(); }

 protected:
  Connection(LinkStrandRef &strand, const string_ &dsid_prefix,
             const string_ &path = "");
  virtual ~Connection() = default;

  HandshakeContext _handshake_context;
  LinkStrandRef _strand;

  // this should rarely be touched
  ref_<Session> _session;

  void destroy_impl() override;

  // path of remote node in local responder
  string_ _path;
  // path of local root node in remote broker
  string_ _remote_path;

  std::vector<MessageRef> _batch_post;
  bool post_message(MessageRef &&msg);

  boost::asio::deadline_timer _deadline;
  virtual void on_deadline_timer_(const boost::system::error_code &error,
                                  shared_ptr_<Connection> connection) {}

 public:
  void start_deadline_timer(size_t seconds);
  void reset_deadline_timer(size_t seconds);

  // server connection
 protected:
  //  void on_server_connect() throw(const std::runtime_error &);

  bool on_receive_f0(MessageRef &&msg);
  bool on_receive_f2(MessageRef &&msg);

  // client connection
 protected:
  string_ _client_token;

  static void on_client_connect(shared_ptr_<Connection> connection) throw(
      const std::runtime_error &);

  void start_client_f0();
  bool on_receive_f1(MessageRef &&msg);
  bool on_receive_f3(MessageRef &&msg);
};

}  // namespace dsa

#endif  // DSA_SDK_CONNECTION_H_
