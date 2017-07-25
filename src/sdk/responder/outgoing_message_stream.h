#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

#include "dsax.h"

namespace dsa {
/**
 * maintain a smart queue of subscription updates
 * this queue works for a single subscription from a single client
 */
class OutgoingMessageStream {
  uint8_t qos_level;
};
}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_
