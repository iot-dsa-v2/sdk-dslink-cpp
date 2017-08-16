#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <deque>
#include <functional>

#include <boost/asio/strand.hpp>
#include <message/error_message.h>

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

 public:
  OutgoingMessageStream(intrusive_ptr_<Session> &&session,
                        uint32_t request_id,
                        size_t unique_id);

  bool is_outgoing() const override { return true; }
  bool is_incoming() const override { return false; }
};

class SubscribeMessageStream : public OutgoingMessageStream {
 private:
  std::deque<SubscribeResponseMessage> _message_queue;
  SubscribeOptions _config;

 public:
  SubscribeMessageStream(intrusive_ptr_<Session> &&session,
                         SubscribeOptions &&config,
                         uint32_t request_id,
                         size_t unique_id);

  void new_message(const SubscribeResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
  StreamType get_type() const override { return StreamType::Subscribe; }
};

class InvokeMessageStream : public OutgoingMessageStream {
 private:
  std::deque<InvokeResponseMessage> _message_queue;
  InvokeOptions _config;

 public:
  InvokeMessageStream(intrusive_ptr_<Session> &&session,
                      InvokeOptions &&config,
                      uint32_t request_id,
                      size_t unique_id);

  void new_message(const InvokeResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
  StreamType get_type() const override { return StreamType::Invoke; }
};

class ListMessageStream : public OutgoingMessageStream {
 private:
  std::deque<ListResponseMessage> _message_queue;
  ListOptions _config;

 public:
  ListMessageStream(intrusive_ptr_<Session> &&session,
                    ListOptions &&config,
                    uint32_t request_id,
                    size_t unique_id);

  void new_message(const ListResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
  StreamType get_type() const override { return StreamType::List; }
};

class SetMessageStream : public OutgoingMessageStream {
 private:
  std::deque<SetResponseMessage> _message_queue;
  SetOptions _config;

 public:
  SetMessageStream(intrusive_ptr_<Session> &&session,
                   SetOptions &&config,
                   uint32_t request_id,
                   size_t unique_id);

  void new_message(const SetResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
  StreamType get_type() const override { return StreamType::Set; }
};

class ErrorMessageStream : public OutgoingMessageStream {
 private:
  std::unique_ptr<Message> _error_message;

 public:
  ErrorMessageStream(intrusive_ptr_<Session> &&session,
                     MessageType type,
                     MessageStatus status,
                     uint32_t request_id = 0);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
  StreamType get_type() const override { return StreamType::Status; }
};

};  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
