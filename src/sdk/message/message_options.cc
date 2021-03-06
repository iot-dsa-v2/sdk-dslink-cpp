#include "dsa_common.h"

#include "message_options.h"

namespace dsa {

BaseRequestOptions::BaseRequestOptions() {}
BaseRequestOptions::BaseRequestOptions(bool priority) : priority(priority) {}

bool BaseRequestOptions::operator==(const BaseRequestOptions& other) const {
  if (other.priority != priority) return false;
  return true;
}

bool BaseRequestOptions::needUpdateOnRemoval(
    const BaseRequestOptions& other) const {
  if (other.priority && priority) return true;

  return false;
}
bool BaseRequestOptions::needUpdateOnChange(
    const BaseRequestOptions& oldopt, const BaseRequestOptions& newopt) const {
  if (oldopt.priority == priority && newopt.priority != priority) return true;

  return false;
}

bool BaseRequestOptions::mergeFrom(const BaseRequestOptions& other) {
  bool changed = false;
  if (other.priority && !priority) {
    priority = true;
    changed = true;
  }
  return changed;
}

const SubscribeOptions SubscribeOptions::default_options;

SubscribeOptions::SubscribeOptions(QosLevel qos, int32_t queue_size,
                                   int32_t queue_time, bool priority)
    : BaseRequestOptions(priority),
      qos(qos),
      queue_size(queue_size),
      queue_duration(queue_time) {}

bool SubscribeOptions::operator==(const SubscribeOptions& other) const {
  if (!BaseRequestOptions::operator==(other)) return false;
  if (other.qos != qos) return false;
  if (other.queue_size != queue_size) return false;
  if (other.queue_duration != queue_duration) return false;
  return true;
}

bool SubscribeOptions::needUpdateOnRemoval(
    const SubscribeOptions& other) const {
  if (BaseRequestOptions::needUpdateOnRemoval(other)) return true;

  if (other.qos == qos && qos != 0) return true;
  if (other.queue_size == queue_size && queue_size != 0) return true;
  if (other.queue_duration == queue_duration && queue_duration != 0) return true;

  return false;
}
bool SubscribeOptions::needUpdateOnChange(
    const SubscribeOptions& oldopt, const SubscribeOptions& newopt) const {
  if (BaseRequestOptions::needUpdateOnChange(oldopt, newopt)) return true;

  if (oldopt.qos == qos && newopt.qos != qos) return true;
  if (oldopt.queue_size == queue_size && newopt.queue_size != queue_size)
    return true;
  if (oldopt.queue_duration == queue_duration && newopt.queue_duration != queue_duration)
    return true;
  return false;
}

bool SubscribeOptions::mergeFrom(const SubscribeOptions& other) {
  bool changed = BaseRequestOptions::mergeFrom(other);

  if (other.qos > qos) {
    qos = other.qos;
    changed = true;
  }
  if (other.queue_size > queue_size) {
    queue_size = other.queue_size;
    changed = true;
  }
  if (other.queue_duration > queue_duration) {
    queue_duration = other.queue_duration;
    changed = true;
  }

  return changed;
}

const ListOptions ListOptions::default_options;

ListOptions::ListOptions() : BaseRequestOptions(false) {}
}  // namespace dsa