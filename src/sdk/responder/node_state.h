#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <vector>

#include <boost/asio/strand.hpp>

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "outgoing_message_stream.h"

namespace dsa {

// maintain streams of a node
class NodeState : public IntrusiveClosable<NodeState> {
 private:
  boost::asio::io_service::strand &_strand;
  std::string _path;
  intrusive_ptr_<NodeModel> _model;
  std::map< uint32_t, intrusive_ptr_<MessageStream> > _subscription_streams;
  std::map< uint32_t, intrusive_ptr_<MessageStream> > _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(boost::asio::io_service::strand &strand, std::string path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }

  //////////////////////////
  // Setters
  //////////////////////////
  void new_subscription_stream(const intrusive_ptr_<Session> &stream,
                               SubscribeOptions &&config,
                               size_t unique_id,
                               uint32_t request_id);
  void remove_subscription_stream(uint32_t request_id);

  void new_list_stream(const intrusive_ptr_<Session> &session,
                       ListOptions &&config,
                       size_t unique_id,
                       uint32_t request_id);
  void remove_list_stream(uint32_t request_id);

  void new_message(const SubscribeResponseMessage &message);
  void set_model(intrusive_ptr_<NodeModel> model) { _model = std::move(model); }
  bool has_model() { return _model != nullptr; }

  void close() override {}
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
