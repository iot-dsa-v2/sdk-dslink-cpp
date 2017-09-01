#include "dsa_common.h"

#include "message_options.h"

namespace dsa {

bool BaseRequestOptions::operator==(BaseRequestOptions& other) const {
  if (other.priority != priority) return false;
  return true;
}

bool BaseRequestOptions::needUpdateOnRemoval(
    const BaseRequestOptions& other) const {
  if (other.priority && priority) return true;

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

bool SubscribeOptions::operator==(SubscribeOptions& other) const {
  if (!BaseRequestOptions::operator==(other)) return false;
  if (other.qos != qos) return false;
  if (other.queue_size != queue_size) return false;
  if (other.queue_time != queue_time) return false;
  return true;
}

bool SubscribeOptions::needUpdateOnRemoval(
    const SubscribeOptions& other) const {
  if (BaseRequestOptions::needUpdateOnRemoval(other)) return true;

  if (other.qos == qos && qos != 0) return true;
  if (other.queue_size == queue_size && queue_size != 0) return true;
  if (other.queue_time == queue_time && queue_time != 0) return true;

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
  if (other.queue_time > queue_time) {
    queue_time = other.queue_time;
    changed = true;
  }

  return changed;
}

}  // namespace dsa