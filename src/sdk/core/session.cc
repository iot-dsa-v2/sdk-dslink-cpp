#include "dsa_common.h"

#include "session.h"

#include "client.h"
#include "crypto/misc.h"
#include "module/logger.h"
#include "server.h"
#include "stream/ack_stream.h"
#include "stream/ping_stream.h"
#include "util/string.h"

namespace dsa {

// logid format : 8 bytes of dsid, and space
//                4 bytes of base64 encoded this pointer,
//                1 byte of reconnection id
static string_ _create_log_id(const string_ &id, void *p) {
  size_t pt_offset = 9;
  string_ rslt;
  if (id.size() > 8) {
    rslt = id.substr(0, 8) + ":xxxx";
  } else {
    rslt = id + ":xxxx";
    pt_offset = id.size() + 1;
  }

  uint8_t *p8 = reinterpret_cast<uint8_t *>(&p);
  // encode 3 bytes of the pointer into base64, and a unique id fo the session
  // instance
  string_ encoded_pointer = base64_encode(p8, 3);

  std::copy(encoded_pointer.begin(), encoded_pointer.end(),
            rslt.begin() + pt_offset);

  return std::move(rslt);
}

Session::Session(const LinkStrandRef &strand, const string_ &dsid)
    : _strand(strand),
      _remote_id(dsid),
      _log_id(_create_log_id(_remote_id, this)),
      requester(*this),
      responder(*this),
      _timer(_strand->add_timer(0, nullptr)),
      _ack_stream(new AckStream(get_ref())),
      _ping_stream(new PingStream(get_ref())) {}

Session::Session(const LinkStrandRef &strand, const string_ &dsid,
                 const string_ &base_path)
    : Session(strand, dsid) {
  _base_path = base_path;
}

Session::~Session() = default;

void Session::set_on_connect(OnConnectCallback &&callback) {
  _on_connect = std::move(callback);
}

void Session::connected(shared_ptr_<Connection> connection) {
  if (_connection != nullptr) {
    remove_ptr_(_connection)->destroy();
  } else if (connection == nullptr) {
    // switch from a nullptr to a nullptr, do nothing
    return;
  }
  _connection = std::move(connection);
  if (_connection != nullptr) {
    if (_connection->get_dsid() != _remote_id) {
      _remote_id = _connection->get_dsid();
      _log_id = _create_log_id(_remote_id, this);
    }

    requester.connected();

    // TODO, what if previous write loop is not finished
    write_loop(get_ref());

    _timer->destroy();
    // start the 15 seconds timer
    _no_receive_in_loop = 0;
    _no_sent_in_loop = 0;
    _timer = _strand->add_timer(15000,
                                [ this, keep_ref = get_ref() ](bool canceled) {
                                  if (canceled) {
                                    return false;
                                  }
                                  return _on_timer();
                                });
  }

  if (_on_connect != nullptr) {
    _on_connect(*this, _connection);
  }
}
void Session::disconnected(const shared_ptr_<Connection> &connection) {
  if (_connection.get() == connection.get()) {
    _connection.reset();
    _timer->destroy();
    requester.disconnected();
    responder.disconnected();
  }
  if (_on_connect != nullptr && _connection == nullptr) {
    // disconnect event
    _on_connect(*this, _connection);
  }
}

void Session::destroy_impl() {
  requester.destroy_impl();
  responder.destroy_impl();
  if (_connection != nullptr) {
    _connection->destroy_in_strand(_connection);
    _connection.reset();
  }
  _ack_stream.reset();
  _ping_stream.reset();
  _timer->destroy();
  _write_streams.clear();
  _pending_acks.clear();
  if (_on_connect != nullptr) {
    _on_connect(*this, _connection);
    _on_connect = nullptr;
  }
}

bool Session::_on_timer() {
  if (_connection == nullptr) {
    return false;
  }
  _no_receive_in_loop++;
  if (_no_receive_in_loop >= 4) {
    // haven't received message in 4 loop (60 seconds)
    _connection->destroy();
    return false;
  }
  _no_sent_in_loop++;
  if (_no_sent_in_loop >= 2) {
    // haven't sent any message in 30 seconds
    // send a ping so connection won't be dropped
    _ping_stream->add_ping();
    _no_sent_in_loop = 0;
  }
  return true;
}

int32_t Session::last_sent_ack() { return _ack_stream->get_ack(); }

void Session::check_pending_acks(int32_t ack) {
  while (!_pending_acks.empty()) {
    AckHolder &first = _pending_acks.front();
    uint32_t d = static_cast<uint32_t>(ack - first.ack);
    if (d < 0x10000000) {
      first.callback(true);
      _pending_acks.pop_front();
    } else {
      return;
    }
  }
}

void Session::receive_message(MessageRef &&message) {
  if (is_destroyed()) return;
  LOG_TRACE(_log_id, LOG << ">> "; message->print_message(LOG););

  _no_receive_in_loop = 0;
  if (message->type() == MessageType::PING) return;

  if (message->type() == MessageType::ACK) {
    check_pending_acks(DOWN_CAST<AckMessage *>(message.get())->get_ack_id());
    return;
  }
  if (message->need_ack()) {
    _ack_stream->add_ack(message->get_ack_id());
  }
  if (message->is_request()) {
    // responder receive request and send response
    responder.receive_message(std::move(message));
  } else {
    // requester send request and receive response
    requester.receive_message(std::move(message));
  }
}

ref_<MessageStream> Session::get_next_ready_stream(int64_t time) {
  while (!_write_streams.empty()) {
    ref_<MessageStream> stream = std::move(_write_streams.front());
    stream->_writing = false;
    _write_streams.pop_front();
    if (stream->peek_next_message_size(0, time) > 0) {
      return std::move(stream);
    }
  }
  return ref_<MessageStream>();
}

size_t Session::peek_next_message(size_t available, int64_t time) {
  while (!_write_streams.empty()) {
    ref_<MessageStream> &stream = _write_streams.front();
    size_t size = stream->peek_next_message_size(available, time);
    if (size > 0) {
      return size;
    }
    stream->_writing = false;
    _write_streams.pop_front();
  }
  return 0;
}

void Session::write_loop(ref_<Session> ref) {
  Connection *connection = ref->_connection.get();
  if (connection == nullptr) {
    // not connected or already destroyed
    ref->_is_writing = false;
    return;
  }

  auto write_buffer = connection->get_write_buffer();
  std::time_t current_time = std::time(nullptr);

  size_t next_message_size =
      ref->peek_next_message(Message::MAX_MESSAGE_SIZE, current_time);

  if (next_message_size == 0) {
    ref->_is_writing = false;
    return;
  }
  ref->_no_sent_in_loop = 0;
  ref->_is_writing = true;

  size_t total_size = 0;
  while (next_message_size > 0 &&
         next_message_size < write_buffer->max_next_size()) {
    auto stream = ref->get_next_ready_stream(current_time);
    AckCallback ack_callback;
    MessageCRef message = stream->get_next_message(ack_callback);

    ++ref->_waiting_ack;
    if (ack_callback != nullptr) {
      ref->_pending_acks.emplace_back(ref->_waiting_ack,
                                      std::move(ack_callback));
    }

    LOG_TRACE(ref->_log_id, LOG << "<<<< ";
              message->print_message(LOG, stream->rid););

    write_buffer->add(*message, stream->rid, ref->_waiting_ack);

    next_message_size =
        ref->peek_next_message(write_buffer->max_next_size(), current_time);
  }

  write_buffer->write([ref = std::move(ref)](
      const boost::system::error_code &error) mutable {
    LinkStrandRef &strand = ref->_strand;
    strand->inject([ref = std::move(ref)]() mutable {
      Session::write_loop(std::move(ref));
    });
  });
}

void Session::write_stream(ref_<MessageStream> &&stream) {
  stream->_writing = true;
  _write_streams.push_back(std::move(stream));
  if (!_is_writing && _connection != nullptr) {
    _is_writing = true;
    _strand->inject([ref = get_ref()]() mutable {
      write_loop(std::move(ref));
    });
  }
}

void Session::write_critical_stream(ref_<MessageStream> &&stream) {
  stream->_writing = true;
  _write_streams.push_front(std::move(stream));
  if (!_is_writing && _connection != nullptr) {
    _is_writing = true;
    _strand->inject([ref = get_ref()]() mutable {
      write_loop(std::move(ref));
    });
  }
}

string_ Session::map_pub_path(const string_ &path) {
  return str_join_path(_base_path, path);
}

}  // namespace dsa
