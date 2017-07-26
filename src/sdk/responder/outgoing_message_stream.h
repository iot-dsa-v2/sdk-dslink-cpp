#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include <deque>

#include "dsa/util.h"

namespace dsa {

// maintain a smart queue of subscription updates
// this queue works for a single subscription from a single client
class OutgoingMessageStream {
  uint8_t _qos;
  std::deque<ValueUpdate> _message_queue;

 public:
  void new_value(ValueUpdate &new_value);
};
}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
