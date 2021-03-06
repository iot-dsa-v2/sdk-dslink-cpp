#ifndef DSA_SDK_SUBSCRIBE_OPTION_H
#define DSA_SDK_SUBSCRIBE_OPTION_H

#if defined(_MSC_VER)
#pragma once
#endif

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
  static const SubscribeOptions default_options;

  // queue size in bytes;
  int32_t queue_size;
  // queue time in seconds
  int32_t queue_duration;

  QosLevel qos;

  SubscribeOptions(QosLevel qos = QosLevel::_0, int32_t queue_size = 0,
                   int32_t queue_time = 0, bool priority = false);

  void make_invalid() { queue_size = -1; }
  bool is_invalid() const { return queue_size == -1; }

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

struct ListOptions : BaseRequestOptions {
  static const ListOptions default_options;

  ListOptions();
  // TODO: implement this
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_OPTION_H
