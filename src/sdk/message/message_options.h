#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#include <cstddef>

namespace dsa {

enum QosLevel : uint8_t {
  _0 = 0,  // only last value
  _1 = 1,  // send all values but drop based on TTL
  _2 = 2,  // send all values and maintain values if connection is dropped
  _3 = 3,  // send all and maintain values even if session is lost and
           // resubscribe is needed
};

struct BaseRequestOptions {
  bool priority = false;

  BaseRequestOptions();
  BaseRequestOptions(bool priority);
  bool operator==(const BaseRequestOptions& other) const;

  bool needUpdateOnRemoval(const BaseRequestOptions& other) const;

  bool needUpdateOnChange(const BaseRequestOptions& oldopt,
                          const BaseRequestOptions& newopt) const;

  // merge from an other option and update self values
  // return true if value is updated;
  bool mergeFrom(const BaseRequestOptions& other);
};

struct SubscribeOptions : BaseRequestOptions {
  // queue size in bytes;
  int32_t queue_size = -1;
  // queue time in seconds
  int32_t queue_time = -1;

  QosLevel qos = QosLevel::_0;

  SubscribeOptions();
  SubscribeOptions(QosLevel qos, int32_t queue_size = 0, int32_t queue_time = 0,
                   bool priority = false);

  bool is_empty() const { return queue_size == -1; }

  bool operator==(const SubscribeOptions& other) const;

  bool operator!=(const SubscribeOptions& other) const {
    return !operator==(other);
  }

  bool needUpdateOnRemoval(const SubscribeOptions& other) const;

  bool needUpdateOnChange(const SubscribeOptions& oldopt,
                          const SubscribeOptions& newopt) const;

  // merge from an other option and update self values
  // return true if value is updated;
  bool mergeFrom(const SubscribeOptions& other);
};

struct InvokeOptions : BaseRequestOptions {
  // TODO: implement this
};

struct SetOptions : BaseRequestOptions {
  // TODO: implement this
};

struct ListOptions : BaseRequestOptions {
  // TODO: implement this
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_OPTION_H
