#ifndef DSA_SDK_OUTGOING_INVOKE_STREAM_H
#define DSA_SDK_OUTGOING_INVOKE_STREAM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include "../message_io_stream.h"

#include "message/response/invoke_response_message.h"
#include "message/message_page_group.h"

namespace dsa {
class InvokeRequestMessage;

class OutgoingInvokeStream final : public MessageQueueStream {
 public:
  typedef std::function<void(OutgoingInvokeStream &,
                             ref_<InvokeRequestMessage> &&)>
      Callback;

 protected:
  Callback _callback;

  std::vector<ref_<InvokeRequestMessage> > _waiting_requests;
  ref_<IncomingPages> _waiting_pages;

  void destroy_impl() final;

 public:
  OutgoingInvokeStream(ref_<Session> &&session, const Path &path, uint32_t rid,
                       ref_<InvokeRequestMessage> &&msg);

  void receive_message(ref_<Message> &&mesage) final;

  void on_request(Callback &&callback);

  void send_response(InvokeResponseMessageCRef &&message);

  void close(MessageStatus status = MessageStatus::CLOSED);

  bool check_close_message(MessageCRef &message) final;

  // return nullptr when page is not ready
  MessageCRef get_first_page_when_ready();
};
}

#endif  // DSA_SDK_OUTGOING_INVOKE_STREAM_H
