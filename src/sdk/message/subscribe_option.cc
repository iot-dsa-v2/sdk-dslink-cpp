#include "dsa_common.h"

#include "subscribe_option.h"

namespace dsa {

SubscribeOption::SubscribeOption(Qos qos, int32_t queue_size,
                                 int32_t queue_time)
    : qos(qos), queue_size(queue_size), queue_time(queue_time) {}

}  // namespace dsa