#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <deque>
#include <functional>

#include "message/error_message.h"

#include "message/response/invoke_response_message.h"
#include "message/response/list_response_message.h"
#include "message/response/set_response_message.h"
#include "message/response/subscribe_response_message.h"

#include "message/base_message.h"
#include "message/message_options.h"
#include "message_io_stream.h"

namespace dsa {
class Session;
class SubscribeResponseMessage;
class InvokeResponseMessage;
class SetResponseMessage;
class ListResponseMessage;

// maintain a smart queue of subscription updates
// this queue works for a single subscription from a single client
class OutgoingMessageStream : public MessageQueueStream {

 public:
  OutgoingMessageStream(ref_<Session> &&session, const std::string &path, uint32_t rid);
};

class OutgoingSubscribeStream : public OutgoingMessageStream {
 public:
  OutgoingSubscribeStream(ref_<Session> &&session, const std::string &path, uint32_t rid);
  void new_message(const SubscribeResponseMessage &new_message);

};
class OutgoingListStream : public OutgoingMessageStream {
public:
  OutgoingListStream(ref_<Session> &&session, const std::string &path, uint32_t rid);

};

//
//class SubscribeMessageStream : public OutgoingMessageStream {
// private:
//  SubscribeOptions _config;
//
// public:
//  SubscribeMessageStream(ref_<Session> &&session,
//                         SubscribeOptions &option, uint32_t rid);
//
//  void new_message(const SubscribeResponseMessage &new_message);
//  void receive_message(MessageRef&& msg){}
//};
//
//class InvokeMessageStream : public OutgoingMessageStream {
// private:
//  InvokeOptions _config;
//
// public:
//  InvokeMessageStream(ref_<Session> &&session, InvokeOptions &option,
//                      uint32_t rid);
//
//  void new_message(const InvokeResponseMessage &new_message);
//  void receive_message(MessageRef&& msg){}
//};
//
//class ListMessageStream : public OutgoingMessageStream {
// private:
//  ListOptions _config;
//
// public:
//  ListMessageStream(ref_<Session> &&session, ListOptions &option,
//                    uint32_t rid);
//
//  void new_message(const ListResponseMessage &new_message);
//  void receive_message(MessageRef&& msg){}
//};
//
//class SetMessageStream : public OutgoingMessageStream {
// private:
//  SetOptions _config;
//
// public:
//  SetMessageStream(ref_<Session> &&session, SetOptions &option,
//                   uint32_t rid);
//
//  //void new_message(const SetResponseMessage &new_message);
//};
//
//class ErrorMessageStream : public OutgoingMessageStream {
// private:
//  MessageRef _error_message;
//
// public:
//  ErrorMessageStream(ref_<Session> &&session, MessageType type,
//                     MessageStatus status, uint32_t rid = 0);
//
//  MessageRef get_next_message() override;
//};

};  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
