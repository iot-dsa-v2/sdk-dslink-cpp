#include "dsa_common.h"

#include "message_options.h"

namespace dsa {

SubscribeOptions::SubscribeOptions(StreamQos qos, int32_t queue_size,
                                 int32_t queue_time)
    : qos(qos), queue_size(queue_size), queue_time(queue_time) {}

}  // namespace dsa