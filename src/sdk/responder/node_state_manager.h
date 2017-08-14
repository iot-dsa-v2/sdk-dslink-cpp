#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include <map>
#include <string>
#include <utility>

#include <boost/asio/strand.hpp>

#include "node_state.h"

namespace dsa {

class NodeStateManager {
 private:
  boost::asio::strand &_strand;
  std::map< std::string, intrusive_ptr_<NodeState> > _node_states;

 public:
  explicit NodeStateManager(boost::asio::io_service::strand &strand);

  const intrusive_ptr_<NodeState> &get_or_create(std::string path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
