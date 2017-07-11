#pragma once

namespace dsa {
namespace responder {

/**
 * maintain a smart queue of subscription updates 
 * this queue works for a single subscription from a single client
 */
class SubscriptionQueue {
  uint8_t qos_level;
  
};


}
}