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
class NodeState : public SharedClosable<NodeState> {
 private:
  boost::asio::io_service::strand _strand;
  std::string _path;
  shared_ptr_<NodeModel> _model;
  std::map<uint32_t, shared_ptr_<SubscribeMessageStream>> _subscription_streams;
  std::map<uint32_t, shared_ptr_<ListMessageStream>> _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(boost::asio::io_service &io_service, std::string path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }

  //////////////////////////
  // Setters
  //////////////////////////
  void add_subscription_stream(
      const shared_ptr_<SubscribeMessageStream> &stream);
  void remove_subscription_stream(uint32_t request_id);

  void add_list_stream(shared_ptr_<ListMessageStream> stream);
  void remove_list_stream(uint32_t request_id);

  void new_message(const SubscribeResponseMessage &message);
  void set_model(const shared_ptr_<NodeModel> &model) { _model = model; }

  void close() override {}
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
