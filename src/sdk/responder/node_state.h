#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <unordered_set>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "outgoing_message_stream.h"

namespace dsa {

class NodeModel;
typedef intrusive_ptr_<NodeModel> ModelPtr;

class MessageStreamHashFunc {
 public:
  size_t operator()(const intrusive_ptr_<MessageStream> &key) const {
    return reinterpret_cast<size_t>(key.get());
  }
};

class MessageStreamKeyCmp {
 public:
  bool operator()(const intrusive_ptr_<MessageStream> &t1,
                  const intrusive_ptr_<MessageStream> &t2) const {
    return t1.get() == t2.get();
  }
};

// maintain streams of a node
class NodeState : public EnableIntrusive<NodeState> {
 private:
  

  LinkStrandPtr strand;
  std::string _path;
  ModelPtr _model;
  std::unordered_set<intrusive_ptr_<SubscribeMessageStream>,
                     MessageStreamHashFunc, MessageStreamKeyCmp>
      _subscription_streams;
  std::unordered_set<intrusive_ptr_<ListMessageStream>, MessageStreamHashFunc,
                     MessageStreamKeyCmp>
      _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(LinkStrandPtr & strand, const std::string &path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }
  bool has_model() { return _model != nullptr; }

  //////////////////////////
  // Setters
  //////////////////////////
  void set_model(ModelPtr model);

  /////////////////////////
  // Other
  /////////////////////////
  void new_message(const SubscribeResponseMessage &message);

  void add_stream(intrusive_ptr_<SubscribeMessageStream> p);
  void add_stream(intrusive_ptr_<ListMessageStream> p);

  void remove_stream(intrusive_ptr_<SubscribeMessageStream> &p);
  void remove_stream(intrusive_ptr_<ListMessageStream> &p);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
