#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <vector>

#include <boost/asio/strand.hpp>

#include "dsa/util.h"
#include "outgoing_message_stream.h"
#include "node_model.h"
#include "message/response/subscribe_response_message.h"

namespace dsa {

// maintain streams of a node
class NodeState : public std::enable_shared_from_this<NodeState> {
 private:
  boost::asio::io_service::strand _strand;
  std::string _path;
  std::shared_ptr<NodeModel> _model;
  std::vector<std::shared_ptr<SubscribeMessageStream>> _subscription_streams;
//  std::vector<std::shared_ptr<OutgoingMessageStream>> _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

  void _handle_new_message(SubscribeResponseMessage message);

 public:
  explicit NodeState(boost::asio::io_service &io_service, std::string path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }

  //////////////////////////
  // Setters
  //////////////////////////
  void add_subscription_stream(std::shared_ptr<SubscribeMessageStream> stream);
//  void add_list_streams(std::shared_ptr<OutgoingMessageStream> stream);
//  void add_invoke_stream(std::shared_ptr<OutgoingMessageStream> stream);
//  void add_set_stream(std::shared_ptr<OutgoingMessageStream> stream);
  void new_message(SubscribeResponseMessage message);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
