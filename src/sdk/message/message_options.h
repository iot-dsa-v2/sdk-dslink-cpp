#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#include <cstddef>

namespace dsa {

enum SubscriptionQos : uint8_t {
  _0 = 0,  // only last value
  _1 = 1,  // send all values but drop based on TTL
  _2 = 2,  // send all values and maintain values if connection is dropped
  _3 = 3,  // send all and maintain values even if session is lost and
           // resubscribe is needed
};

struct BaseRequestOptions {
  bool priority = false;

  bool operator==(BaseRequestOptions& other) const;

  bool needUpdateOnRemoval(const BaseRequestOptions& options) const;

  // merge from an other option and update self values
  // return true if value is updated;
  bool mergeFrom(const BaseRequestOptions& options);
};

struct SubscribeOptions : BaseRequestOptions {
  // queue size in bytes;
  int32_t queue_size = 0;
  // queue time in seconds
  int32_t queue_time = 0;

  SubscriptionQos qos = SubscriptionQos::_0;

  bool operator==(SubscribeOptions& other) const;

  bool needUpdateOnRemoval(const SubscribeOptions& options) const;

  // merge from an other option and update self values
  // return true if value is updated;
  bool mergeFrom(const SubscribeOptions& options);
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
