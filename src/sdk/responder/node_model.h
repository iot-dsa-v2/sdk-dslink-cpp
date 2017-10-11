#ifndef DSA_SDK_NODE_MODEL_MANAGER_H_
#define DSA_SDK_NODE_MODEL_MANAGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>

#include "core/link_strand.h"
#include "message/message_options.h"
#include "message/response/subscribe_response_message.h"
#include "util/enable_intrusive.h"
#include "util/path.h"

namespace dsa {

class NodeState;

class NodeModel : public ClosableRef<NodeModel> {
  friend class NodeState;

 public:
  typedef std::function<void(SubscribeResponseMessageCRef &&)>
      SubscribeCallback;

 protected:
  LinkStrandRef _strand;

  ref_<NodeState> _state;

  SubscribeCallback _subscribe_callback;
  SubscribeResponseMessageCRef _cached_value;

 public:
  static NodeModel *WAITING;
  static NodeModel *INVALID;
  static NodeModel *UNAVAILABLE;

  explicit NodeModel(LinkStrandRef strand);
  virtual ~NodeModel();

  virtual void initialize() {}

  ref_<NodeModel> get_child(const std::string &name);

  ref_<NodeModel> add_child(const std::string &name, ref_<NodeModel> model);

  // when return true, model will be removed
  virtual bool periodic_check(size_t ts) { return true; }
  virtual bool allows_runtime_child_change() { return false; }
  virtual ref_<NodeModel> on_demand_create_child(const Path &path) {
    return INVALID;
  }

  void subscribe(const SubscribeOptions &options, SubscribeCallback &&callback);
  void unsubscribe();

  void set_value(Variant &&value);
  void set_value(MessageValue &&value);
  void set_message(SubscribeResponseMessageCRef &&message);

  virtual void on_subscribe(const SubscribeOptions &options){};
  virtual void on_subscribe_option_change(const SubscribeOptions &options){};
  virtual void on_unsubscribe(){};
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_
