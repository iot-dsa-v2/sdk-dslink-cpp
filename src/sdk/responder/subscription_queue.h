#ifndef DSA_SDK_SUBSCRIPTION_QUEUE_H_
#define DSA_SDK_SUBSCRIPTION_QUEUE_H_

namespace dsa {
/**
 * maintain a smart queue of subscription updates
 * this queue works for a single subscription from a single client
 */
class SubscriptionQueue {
  uint8_t qos_level;
};
}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIPTION_QUEUE_H_