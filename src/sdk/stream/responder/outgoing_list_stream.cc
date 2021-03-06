#include "dsa_common.h"

#include "outgoing_list_stream.h"

#include "core/session.h"
#include "message/response/list_response_message.h"
#include "module/logger.h"
#include "util/little_endian.h"
#include "variant/variant.h"

namespace dsa {
OutgoingListStream::OutgoingListStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid,
                                       ListOptions &&options)
    : MessageRefedStream(std::move(session), path, rid) {}

void OutgoingListStream::on_list_close(ListCloseCallback &&callback) {
  _close_callback = std::move(callback);
}

void OutgoingListStream::destroy_impl() {
  if (_close_callback != nullptr) {
    _close_callback(*this);
    _close_callback = nullptr;
  };
}

void OutgoingListStream::update_list_value(const string_ &key,
                                           const VarBytesRef &value) {
  _cached_map[key] = value;
  send_message();
}
void OutgoingListStream::update_response_status(Status status) {
  if (_status != status) {
    _status_changed = true;
    _status = status;
    send_message();
  }
}
void OutgoingListStream::update_list_refreshed() {
  _refreshed = true;
  _cached_map.clear();
}
void OutgoingListStream::update_list_pub_path(const string_ &path) {
  if (!path.empty()) {
    _pending_pub_path = path;
    send_message();
  }
}
size_t OutgoingListStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (_cached_map.empty() && !_status_changed) return 0;

  size_t size = StaticHeaders::TOTAL_SIZE;

  if (_refreshed) {
    size++;
  }
  if (_status != Status::OK) {
    size += 2;
  }
  if (!_pending_pub_path.empty()) {
    size += _pending_pub_path.size() + 3;  // type 1B + size 2B
  }
  // TODO: count the length for other dynamic headers;
  if (!_cached_map.empty()) {
    auto it = _cached_map.begin();
    size += it->first.size() + it->second->size() + 4;

    if (size > available) return size;

    for (++it; it != _cached_map.end(); ++it) {
      size_t this_size = it->first.size() + it->second->size() + 4;
      if (this_size + size > available) {
        break;
      }
      size += this_size;
    }
  }
  _next_size = size;
  return size;
}
MessageCRef OutgoingListStream::get_next_message(AckCallback &) {
  ListResponseMessage *message = new ListResponseMessage();
  if (_status != Status::OK) {
    message->set_status(_status);
  }
  _status_changed = false;
  if (_refreshed) {
    message->set_refreshed(true);
    _refreshed = false;
  }
  if (!_pending_pub_path.empty()) {
    message->set_pub_path(_pending_pub_path);
    _pending_pub_path.clear();
  }

  size_t available_size = _next_size - message->size();

  RefCountBytes body{available_size};
  size_t pos = 0;
  for (auto it = _cached_map.begin(); it != _cached_map.end();) {
    auto &bytes = it->second->get_bytes();
    size_t this_size = it->first.size() + bytes->size() + 4;
    if (this_size <= available_size) {
      available_size -= this_size;
      // write key
      pos += write_str_with_len(body.data() + pos, it->first);
      // write value
      pos += write_16_t(body.data() + pos, bytes->size());

      std::copy(bytes->begin(), bytes->end(), body.data() + pos);
      pos += bytes->size();

      // remove and move to next iterator
      it = _cached_map.erase(it);
    } else {
      break;
    }
  }
  //
  body.resize(pos);
  message->set_body(std::move(body));
  return message->get_ref();
}

void OutgoingListStream::receive_message(ref_<Message> &&message) {
  LOG_ERROR(__FILENAME__,
            LOG << "unexpected request update on list stream");
}
}
