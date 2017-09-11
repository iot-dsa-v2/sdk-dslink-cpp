#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#include <map>
#include <memory>
#include <string>

#include "util/enable_intrusive.h"
#include "util/path.h"
#include "message/message_options.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {


class NodeModel : public EnableRef<NodeModel> {
  typedef std::function<void(SubscribeResponseMessageCRef &&)>
      SubscribeCallback;

 protected:
  SubscribeCallback _subscribe_callback;

 public:
  static const ref_<NodeModel> WAITING_REF;
  static const ref_<NodeModel> INVALID_REF;
  static const ref_<NodeModel> UNAVAILIBLE_REF;

  virtual void subscribe(const SubscribeOptions &options,
                         SubscribeCallback &&callback);
};

class NodeModelManager {
 public:
  // should immediately return the node model if it exists
  // return nullptr is model doesn't exist
  ref_<NodeModel> &get_model(const Path &path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
