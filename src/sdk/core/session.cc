#include "dsa_common.h"

#include "session.h"

#include "client.h"
#include "crypto/hash.h"
#include "crypto/misc.h"
#include "module/logger.h"
#include "server.h"
#include "stream/ack_stream.h"

namespace dsa {

ClientSessions::ClientSessions(const ClientInfo &info) : _info(info) {
  gen_salt(reinterpret_cast<uint8_t *>(&_session_id_seed), sizeof(uint64_t));
};

void ClientSessions::add_session(LinkStrandRef &strand,
                                 const string_ &session_id,
                                 GetSessionCallback &&callback) {
  auto search = _sessions.find(session_id);
  if (search != _sessions.end()) {
    callback(search->second);
    return;
  }
  string_ sid = get_new_session_id(session_id);
  auto session = make_ref_<Session>(strand->get_ref(), sid);

  _sessions[sid] = session;

  callback(session);
}

string_ ClientSessions::get_new_session_id(const string_ old_session_id) {
  Hash hash("sha256");

  std::vector<uint8_t> data(16);
  memcpy(&data[0], &_session_id_seed, sizeof(uint64_t));
  memcpy(&data[8], &_session_id_count, sizeof(uint64_t));
  _session_id_count++;

  hash.update(data);

  string_ result = base64_url_convert(hash.digest_base64());
  if (result != old_session_id || _sessions.find(result) == _sessions.end()) {
    return std::move(result);
  }
  return get_new_session_id(old_session_id);
}

void ClientSessions::destroy() {
  for (auto &kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->destroy();
    }
  }
  _sessions.clear();
}

Session::Session(LinkStrandRef strand, const string_ &session_id)
    : _strand(std::move(strand)),
      _session_id(session_id),
      requester(*this),
      responder(*this),
      _ack_stream(new AckStream(get_ref())) {}

Session::~Session() = default;

void Session::set_on_connected(OnConnectedCallback &&callback, uint8_t type) {
  on_connected_type = type;
  on_connected = std::move(callback);
}

void Session::connected(shared_ptr_<Connection> connection) {
  if (_connection != nullptr) {
    _connection->destroy();
  }
  _connection = std::move(connection);
  write_loop(get_ref());
  if (on_connected != nullptr) {
    // TODO, update ClientConnetionData
    if (on_connected_type == FIRST_CONNECTION) {
      std::move(on_connected)(connection_data);
    } else {
      // TODO, implement other connection type
    }
  }
}

void Session::destroy_impl() {
  requester.destroy_impl();
  responder.destroy_impl();
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _ack_stream.reset();
}

void Session::disconnected(const shared_ptr_<Connection> &connection) {
  if (_connection.get() == connection.get()) {
    _connection.reset();
  }
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
  LOG_TRACE(_strand->logger(), LOG << "receive message: " << message->type());

  if (message->need_ack()) {
    _ack_stream->add_ack(message->get_ack_id());
  }
  if (message->type() == MessageType::ACK) {
    check_pending_acks(DOWN_CAST<AckMessage *>(message.get())->get_ack_id());
    return;
  }
  if (message->is_request()) {
    // responder receive request and send response
    responder.receive_message(std::move(message));
  } else {
    // requester sent request and receive response
    requester.receive_message(std::move(message));
  }
}

ref_<MessageStream> Session::get_next_ready_stream(int64_t time) {
  while (!_write_streams.empty()) {
    ref_<MessageStream> stream = std::move(_write_streams.front());
    _write_streams.pop_front();
    if (stream->peek_next_message_size(0, time) > 0) {
      return std::move(stream);
    }
  }
  return ref_<MessageStream>();
}

size_t Session::peek_next_message(size_t availible, int64_t time) {
  while (!_write_streams.empty()) {
    ref_<MessageStream> &stream = _write_streams.front();
    size_t size = stream->peek_next_message_size(availible, time);
    if (size > 0) {
      return size;
    }
    _write_streams.pop_front();
  }
  return 0;
}

void Session::write_loop(ref_<Session> sthis) {
  Connection *connection = sthis->_connection.get();
  if (connection == nullptr) {
    // not connected or already destroyed
    sthis->_is_writing = false;
    return;
  }

  auto write_buffer = connection->get_write_buffer();
  std::time_t current_time = std::time(nullptr);

  size_t next_message_size =
      sthis->peek_next_message(Message::MAX_MESSAGE_SIZE, current_time);

  if (next_message_size == 0) {
    sthis->_is_writing = false;
    return;
  }

  sthis->_is_writing = true;

  size_t total_size = 0;
  while (next_message_size > 0 &&
         next_message_size < write_buffer->max_next_size()) {
    auto stream = sthis->get_next_ready_stream(current_time);
    AckCallback ack_callback;
    MessageCRef message = stream->get_next_message(ack_callback);

    ++sthis->_waiting_ack;
    if (ack_callback != nullptr) {
      sthis->_pending_acks.emplace_back(
          AckHolder(sthis->_waiting_ack, std::move(ack_callback)));
    }

    LOG_TRACE(sthis->_strand->logger(),
              LOG << "send message: " << message->type());

    write_buffer->add(*message, stream->rid, sthis->_waiting_ack);

    next_message_size =
        sthis->peek_next_message(write_buffer->max_next_size(), current_time);
  }

  write_buffer->write([sthis = std::move(sthis)](
      const boost::system::error_code &error) mutable {
    LinkStrandRef strand = sthis->_strand;
    strand->dispatch([sthis = std::move(sthis)]() mutable {
      Session::write_loop(std::move(sthis));
    });
  });
}

void Session::write_stream(ref_<MessageStream> &&stream) {
  _write_streams.push_back(std::move(stream));
  if (!_is_writing && _connection != nullptr) {
    write_loop(get_ref());
  }
}

}  // namespace dsa
