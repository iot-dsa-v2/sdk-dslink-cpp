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
class NodeState : public InheritableEnableShared<NodeState> {
 private:
  boost::asio::io_service::strand _strand;
  std::string _path;
  std::shared_ptr<NodeModel> _model;
  std::map<uint32_t, std::shared_ptr<SubscribeMessageStream>> _subscription_streams;
  std::map<uint32_t, std::shared_ptr<ListMessageStream>> _list_streams;
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
  void add_subscription_stream(const std::shared_ptr<SubscribeMessageStream> &stream);
  void remove_subscription_stream(uint32_t request_id);

  void add_list_stream(std::shared_ptr<ListMessageStream> stream);
  void remove_list_stream(uint32_t request_id);

  void new_message(const SubscribeResponseMessage &message);
  void set_model(const std::shared_ptr<NodeModel> &model) { _model = model; }
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
