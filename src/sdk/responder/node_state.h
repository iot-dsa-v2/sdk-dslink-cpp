#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <vector>

#include <boost/asio/strand.hpp>

#include "message/response/subscribe_response_message.h"
#include "node_model.h"
#include "outgoing_message_stream.h"
#include "core/message_stream.h"

namespace dsa {

// maintain streams of a node
class NodeState : public StreamHolder {
 private:
  typedef intrusive_ptr_<NodeModel> model_ptr_;

  boost::asio::strand &_strand;
  std::string _path;
  model_ptr_ _model;
  std::map< size_t, stream_ptr_ > _subscription_streams;
  std::map< size_t, stream_ptr_ > _list_streams;
  std::unique_ptr<SubscribeResponseMessage> _last_value;

 public:
  explicit NodeState(boost::asio::io_service::strand &strand, std::string path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }
  bool has_model() { return _model != nullptr; }

  //////////////////////////
  // Setters
  //////////////////////////
  void set_model(model_ptr_ model) { _model = std::move(model); }

  /////////////////////////
  // Other
  /////////////////////////
  void new_message(const SubscribeResponseMessage &message);

  void close() override {}

  void add_stream(const stream_ptr_ &stream) override;
  void remove_stream(const MessageStream *stream) override;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
