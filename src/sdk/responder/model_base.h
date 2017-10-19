#ifndef DSA_SDK_MODEL_BASE_H_
#define DSA_SDK_MODEL_BASE_H_

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
class OutgoingListStream;
class OutgoingInvokeStream;
class OutgoingSetStream;

class NodeState;

class NodeModelBase;
typedef ref_<NodeModelBase> ModelRef;

class NodeModelBase : public ClosableRef<NodeModelBase> {
  friend class NodeState;

 public:
  typedef std::function<void(SubscribeResponseMessageCRef &&)>
      SubscribeCallback;

 protected:
  LinkStrandRef _strand;

  ref_<NodeState> _state;

  void close_impl() override;

  virtual void initialize() {}

 public:
  static ModelRef WAITING;
  static ModelRef INVALID;
  static ModelRef UNAVAILABLE;

  explicit NodeModelBase(LinkStrandRef &&strand);
  virtual ~NodeModelBase();

  ModelRef get_child(const std::string &name);

  ModelRef add_child(const std::string &name, ModelRef &&model);

  // when return true, close() will be called by NodeState
  // and model will be removed from the node tree
  virtual bool periodic_check(size_t ts) { return false; }

  virtual bool allows_runtime_child_change() { return false; }
  virtual ModelRef on_demand_create_child(const Path &path) { return INVALID; }


  /// subscribe
protected:
  SubscribeCallback _subscribe_callback;
  SubscribeResponseMessageCRef _cached_value;
public:
  void subscribe(const SubscribeOptions &options, SubscribeCallback &&callback);
  void unsubscribe();

  void set_value(Variant &&value);
  void set_value(MessageValue &&value);
  void set_message(SubscribeResponseMessageCRef &&message);

  virtual void on_subscribe(const SubscribeOptions &options){};
  virtual void on_subscribe_option_change(const SubscribeOptions &options){};
  virtual void on_unsubscribe(){};

  /// list
public:
  virtual void init_list_stream(OutgoingListStream &stream){};

  /// invoke
public:
  virtual void on_invoke(ref_<OutgoingInvokeStream> &&stream);

  /// invoke
public:
  virtual void on_set(ref_<OutgoingSetStream> &&stream);
};

}  // namespace dsa

#endif  // DSA_SDK_MODEL_BASE_H_
