#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#include <map>
#include <memory>
#include <string>

#include "message/message_options.h"
#include "message/response/subscribe_response_message.h"
#include "util/enable_intrusive.h"
#include "util/path.h"

namespace dsa {

class NodeState;

class NodeModel : public EnableRef<NodeModel> {
 public:
  typedef std::function<void(SubscribeResponseMessageCRef &&)>
      SubscribeCallback;

 protected:
  ref_<NodeState> _state;

  SubscribeCallback _subscribe_callback;

 public:
  static const ref_<NodeModel> WAITING_REF;
  static const ref_<NodeModel> INVALID_REF;
  static const ref_<NodeModel> UNAVAILIBLE_REF;

  virtual ~NodeModel();

  ref_<NodeModel> get_child(const std::string &name);

  virtual ref_<NodeModel> on_demand_create_child(const std::string &name) {
    return INVALID_REF;
  }

  virtual void subscribe(const SubscribeOptions &options,
                         SubscribeCallback &&callback);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
