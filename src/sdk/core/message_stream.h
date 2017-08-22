#ifndef DSA_SDK_NETWORK_STREAM_H
#define DSA_SDK_NETWORK_STREAM_H

#include <map>

#include "core/link_strand.h"

#include "message/base_message.h"
#include "util/enable_intrusive.h"

namespace dsa {

class Session;
class Message;
class MessageStream;
class StreamInfo;

enum StreamType {
  Subscribe,
  List,
  Invoke,
  Set,
  Status
};



class MessageStream : public EnableIntrusive<MessageStream> {
 protected:
  LinkStrandPtr _strand;

  bool _closed{false};
  std::function<void()> _set_ready;

 public:
  const uint32_t _request_id;
  const size_t _unique_id;

  MessageStream(intrusive_ptr_<Session> &&session,
                uint32_t request_id,
                size_t unique_id);
  virtual ~MessageStream() = default;


  virtual StreamType get_type() const = 0;
  virtual bool is_outgoing() const = 0;
  virtual bool is_incoming() const = 0;
  virtual size_t get_next_message_size() = 0;
  virtual const Message &get_next_message() = 0;
  bool is_closed() const { return _closed; }

  virtual void close_stream();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_STREAM_H
