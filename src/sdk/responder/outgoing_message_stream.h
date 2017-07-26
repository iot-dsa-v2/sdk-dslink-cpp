#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <functional>
#include <deque>
#include <mutex>

#include <boost/asio/strand.hpp>

#include "dsa/util.h"

namespace dsa {
class Session;

// maintain a smart queue of subscription updates
// this queue works for a single subscription from a single client
class OutgoingMessageStream {
  using io_service = boost::asio::io_service;

  std::mutex _key;
  uint8_t _qos;
  unsigned int _id;
  std::function<void()> _set_ready;
  std::deque<ValueUpdate> _message_queue;
  boost::asio::io_service::strand &_strand;

 public:

  OutgoingMessageStream(const std::shared_ptr<Session> &session, uint8_t qos, unsigned int id);

  void new_value(ValueUpdate &new_value);

//   get_value();

};
}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
