#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#include <cstddef>

namespace dsa {
struct SubscribeOption {
  enum Qos : uint8_t {
    Qos0 = 0, // only last value
    Qos1 = 1, // send all values but drop based on TTL
    Qos2 = 2, // send all values and maintain values if connection is dropped
    Qos3 = 3, // send all and maintain values even if session is lost and resubscribe is needed
  };

  Qos qos;
  // queue size in bytes;
  int32_t queue_size;
  // queue time in seconds
  int32_t queue_time;

  SubscribeOption(Qos qos, int32_t queue_size, int32_t queue_time);
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_OPTION_H
