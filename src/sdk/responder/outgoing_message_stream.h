#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <deque>
#include <functional>

#include <boost/asio/strand.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "dsa/util.h"
#include "message/response/subscribe_response_message.h"
#include "network/message_stream.h"

namespace dsa {
class Session;

// maintain a smart queue of subscription updates
// this queue works for a single subscription from a single client
class OutgoingMessageStream : public MessageStream {
 protected:
  using io_service = boost::asio::io_service;

  std::function<void()> _set_ready;
  boost::shared_mutex _key;
  uint8_t _qos;

 public:
  OutgoingMessageStream(const std::shared_ptr<Session> &session, uint8_t qos,
                        size_t id, uint32_t rid)
      : MessageStream(session, rid, id), _qos(qos) {}
};

class SubscribeMessageStream : public OutgoingMessageStream {
 private:
  std::deque<SubscribeResponseMessage> _message_queue;

 public:
  SubscribeMessageStream(const std::shared_ptr<Session> &session, uint8_t qos, size_t id, uint32_t rid);

  void new_message(const SubscribeResponseMessage &new_message);

  size_t get_next_message_size() override;
  const Message &get_next_message() override;
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
