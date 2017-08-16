#ifndef DSA_SDK_NETWORK_STREAM_H
#define DSA_SDK_NETWORK_STREAM_H

#include <boost/asio/strand.hpp>
#include <map>

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

class StreamHolder : public IntrusiveClosable<StreamHolder> {
 protected:
  typedef intrusive_ptr_<MessageStream> stream_ptr_;

 public:
  virtual void add_stream(const stream_ptr_ &stream) = 0;
  virtual void remove_stream(const MessageStream *raw_stream_ptr) = 0;
  virtual ~StreamHolder() = default;
};

class MessageStream : public EnableIntrusive<MessageStream> {
 protected:
  boost::asio::io_service::strand &_strand;
  std::vector<intrusive_ptr_<StreamHolder>> _holders;
  std::atomic_bool _closed{false};
  std::function<void()> _set_ready;

 public:
  const uint32_t _request_id;
  const size_t _unique_id;

  MessageStream(intrusive_ptr_<Session> &&session,
                uint32_t request_id,
                size_t unique_id);
  virtual ~MessageStream() = default;

  void add_holder(intrusive_ptr_<StreamHolder> holder);

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
