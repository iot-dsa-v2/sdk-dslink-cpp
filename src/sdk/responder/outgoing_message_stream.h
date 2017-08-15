#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <deque>
#include <functional>

#include <boost/asio/strand.hpp>

#include "message/response/subscribe_response_message.h"
#include "message/response/set_response_message.h"
#include "message/response/invoke_response_message.h"
#include "message/response/list_response_message.h"

#include "message/message_options.h"
#include "core/message_stream.h"

namespace dsa {
class Session;
class SubscribeResponseMessage;
class InvokeResponseMessage;
class SetResponseMessage;
class ListResponseMessage;

// maintain a smart queue of subscription updates
// this queue works for a single subscription from a single client
class OutgoingMessageStream : public MessageStream {
 protected:
  using io_service = boost::asio::io_service;

  std::function<void()> _set_ready;

 public:
  OutgoingMessageStream(intrusive_ptr_<Session> session, _stream_container container, size_t id, uint32_t rid);
};

class SubscribeMessageStream : public OutgoingMessageStream {
 private:
  std::deque<SubscribeResponseMessage> _message_queue;
  SubscribeOptions _config;

 public:
  SubscribeMessageStream(intrusive_ptr_<Session> session,
                         _stream_container container,
                         SubscribeOptions config,
                         size_t id,
                         uint32_t rid);

  void new_message(const SubscribeResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;

};

class InvokeMessageStream : public OutgoingMessageStream {
 private:
  std::deque<InvokeResponseMessage> _message_queue;
  InvokeOptions _config;

 public:
  InvokeMessageStream(intrusive_ptr_<Session> session,
                      _stream_container container,
                      InvokeOptions config,
                      size_t id,
                      uint32_t rid);

  void new_message(const InvokeResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
};

class ListMessageStream : public OutgoingMessageStream {
 private:
  std::deque<ListResponseMessage> _message_queue;
  ListOptions _config;

 public:
  ListMessageStream(intrusive_ptr_<Session> session,
                    _stream_container container,
                    ListOptions config,
                    size_t id,
                    uint32_t rid);

  void new_message(const ListResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
};

class SetMessageStream : public OutgoingMessageStream {
 private:
  std::deque<SetResponseMessage> _message_queue;
  SetOptions _config;

 public:
  SetMessageStream(intrusive_ptr_<Session> session,
                   _stream_container container,
                   SetOptions config,
                   size_t id,
                   uint32_t rid);

  void new_message(const SetResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
