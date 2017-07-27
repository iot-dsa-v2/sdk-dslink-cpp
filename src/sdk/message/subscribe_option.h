#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#include <cstddef>
namespace dsa {

struct SubscribeOption {
  enum Qos : uint8_t {
    Qos0 = 0,
    Qos1 = 1,
    Qos2 = 2,
    Qos3 = 3,
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
