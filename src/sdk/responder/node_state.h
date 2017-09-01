#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <unordered_set>

#include "core/link_strand.h"

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "stream/outgoing_message_stream.h"

namespace dsa {

class NodeModel;
typedef ref_<NodeModel> ModelRef;

class MessageStreamHashFunc {
 public:
  size_t operator()(const ref_<MessageStream> &key) const {
    return reinterpret_cast<size_t>(key.get());
  }
};

class MessageStreamKeyCmp {
 public:
  bool operator()(const ref_<MessageStream> &t1,
                  const ref_<MessageStream> &t2) const {
    return t1.get() == t2.get();
  }
};

// maintain streams of a node
class NodeState : public EnableRef<NodeState> {
 private:
  LinkStrandRef strand;
  std::string _path;
  ModelRef _model;
  std::unordered_set<ref_<OutgoingSubscribeStream>, MessageStreamHashFunc,
                     MessageStreamKeyCmp>
      _subscription_streams;
  std::unordered_set<ref_<OutgoingListStream>, MessageStreamHashFunc,
                     MessageStreamKeyCmp>
      _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(LinkStrandRef &strand, const std::string &path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }
  bool has_model() { return _model != nullptr; }

  //////////////////////////
  // Setters
  //////////////////////////
  void set_model(ModelRef model);

  /////////////////////////
  // Other
  /////////////////////////
  void new_message(const SubscribeResponseMessage &message);

  void add_stream(ref_<OutgoingSubscribeStream> p);
  void add_stream(ref_<OutgoingListStream> p);

  void remove_stream(ref_<OutgoingSubscribeStream> &p);
  void remove_stream(ref_<OutgoingListStream> &p);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
