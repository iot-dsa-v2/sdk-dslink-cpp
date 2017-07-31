#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#include <cstddef>

namespace dsa {

enum StreamQos : uint8_t {
  _0 = 0, // only last value
  _1 = 1, // send all values but drop based on TTL
  _2 = 2, // send all values and maintain values if connection is dropped
  _3 = 3, // send all and maintain values even if session is lost and resubscribe is needed
};

struct SubscribeOptions {
  StreamQos qos;
  // queue size in bytes;
  int32_t queue_size;
  // queue time in seconds
  int32_t queue_time;

  SubscribeOptions(StreamQos qos, int32_t queue_size, int32_t queue_time);
};

struct InvokeOptions {
  // TODO: implement this
};

struct SetOptions {
  // TODO: implement this
};

struct ListOptions {
  // TODO: implement this
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_OPTION_H
