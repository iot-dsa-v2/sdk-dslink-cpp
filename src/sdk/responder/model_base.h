#ifndef DSA_SDK_MODEL_BASE_H_
#define DSA_SDK_MODEL_BASE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <string>

#include "core/link_strand.h"
#include "message/message_options.h"
#include "message/response/subscribe_response_message.h"
#include "util/enable_ref.h"
#include "util/path.h"

namespace dsa {
class OutgoingInvokeStream;
class OutgoingSetStream;

class NodeState;
class NodeModel;

class NodeModelBase;
typedef ref_<NodeModelBase> ModelRef;

class NodeModelBase : public DestroyableRef<NodeModelBase> {
  friend class NodeState;
  friend class NodeModel;

 protected:
  LinkStrandRef _strand;

  ref_<NodeState> _state;

  void destroy_impl() override;

  virtual void initialize() {}

 public:
  static ModelRef WAITING;
  static ModelRef INVALID;
  static ModelRef UNAVAILABLE;

  explicit NodeModelBase(LinkStrandRef &&strand);
  virtual ~NodeModelBase();

  ModelRef get_child(const string_ &name);

  ModelRef add_child(const string_ &name, ModelRef &&model);

  // when return true, destroy() will be called by NodeState
  // and model will be removed from the node tree
  virtual bool periodic_check(size_t ts) { return false; }

  virtual bool allows_runtime_child_change() { return false; }
  virtual ModelRef on_demand_create_child(const Path &path) { return INVALID; }

  /// subscribe
 protected:
  bool _need_subscribe = false;
  SubscribeResponseMessageCRef _cached_value;
  virtual void on_subscribe(const SubscribeOptions &options,
                            bool first_request){};
  virtual void on_unsubscribe() {}
  void subscribe(const SubscribeOptions &options);
  void unsubscribe();

 public:
  void set_value(Var &&value);
  void set_value(MessageValue &&value);
  void set_subscribe_response(SubscribeResponseMessageCRef &&message);

  /// list
 protected:
  bool _need_list = false;
  virtual void on_list(BaseOutgoingListStream &stream, bool first_request){};
  virtual void on_unlist() {}

  void list(BaseOutgoingListStream &stream);
  void unlist();

  // get the summery Map when it's listed in a parent node
  virtual VarBytesRef &get_summary();

  /// invoke
 public:
  virtual void invoke(ref_<OutgoingInvokeStream> &&stream,
                      ref_<NodeState> &parent);

  /// set
 public:
  virtual void set(ref_<OutgoingSetStream> &&stream);
};

}  // namespace dsa

#endif  // DSA_SDK_MODEL_BASE_H_
