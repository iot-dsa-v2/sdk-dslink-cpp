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
  
  // queue size in bytes;
  int32_t queue_size;
  // queue time in seconds
  int32_t queue_time;

  StreamQos qos;

  bool priority;

  SubscribeOptions(StreamQos qos = StreamQos::_0, int32_t queue_size = 0, int32_t queue_time = 0, bool priority = false);

  // merge from an other option and update self values
  // return true if value is updated;
  bool mergeFrom(const SubscribeOptions & options);
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
