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

void OutgoingListStream::on_close(CloseCallback &&callback) {
  _close_callback = std::move(callback);
}

void OutgoingListStream::destroy_impl() {
  if (_close_callback != nullptr) {
    std::move(_close_callback)(*this);
  };
}

void OutgoingListStream::update_value(const std::string &key, BytesRef &value) {
  _cached_map[key] = value;
  send_message();
}
void OutgoingListStream::update_value(const std::string &key, const Var &v) {
  _cached_map[key] = make_ref_<const IntrusiveBytes>(std::move(v.to_msgpack()));
  send_message();
}

size_t OutgoingListStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (_cached_map.empty()) return 0;

  size_t size = StaticHeaders::TOTAL_SIZE;
  // TODO: dynamic headers; base_path;

  auto it = _cached_map.begin();
  size += it->first.size() + it->second->size() + 4;
  if (size > available) return size;

  for (; it != _cached_map.end(); ++it) {
    size_t this_size = it->first.size() + it->second->size() + 4;
    if (this_size + size > available) {
      break;
    }
    size += this_size;
  }
  _next_size = size;
  return size;
}
MessageCRef OutgoingListStream::get_next_message(AckCallback &) {
  _writing = false;

  ListResponseMessage *message = new ListResponseMessage();
  // TODO: dynamic headers; base_path;
  size_t availible_size = _next_size - message->size();

  IntrusiveBytes body{availible_size};
  size_t pos = 0;
  for (auto it = _cached_map.begin(); it != _cached_map.end();) {
    size_t this_size = it->first.size() + it->second->size() + 4;
    if (this_size <= availible_size) {
      availible_size -= this_size;
      // write key
      pos += write_str_with_len(body.data() + pos, it->first);
      // write value
      pos += write_16_t(body.data() + pos, it->second->size());

      std::copy(it->second->begin(), it->second->end(), body.data() + pos);
      pos += it->second->size();

      // remove and move to next iterator
      it = _cached_map.erase(it);
    } else {
      break;
    }
  }
  message->set_body(new IntrusiveBytes(std::move(body)));
  return message->get_ref();
}

void OutgoingListStream::receive_message(MessageCRef &&mesage) {
  LOG_ERROR(_session->get_strand()->logger(),
            LOG << "unexpected request update on list stream");
}
}
