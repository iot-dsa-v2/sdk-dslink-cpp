#include "dsa_common.h"

#include "message_options.h"

namespace dsa {

SubscribeOptions::SubscribeOptions(StreamQos qos, int32_t queue_size,
                                   int32_t queue_time, bool priority)
    : qos(qos),
      queue_size(queue_size),
      queue_time(queue_time),
      priority(priority) {}

bool SubscribeOptions::mergeFrom(const SubscribeOptions& other) {
  bool changed = false;
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
  if (other.priority && !priority) {
    priority = true;
    changed = true;
  }
  return changed;
}

}  // namespace dsa