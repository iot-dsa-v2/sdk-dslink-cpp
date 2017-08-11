#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include <map>
#include <string>
#include <utility>

#include <boost/asio/io_service.hpp>

#include "core/app.h"
#include "node_state.h"

namespace dsa {
class App;
/**
 * maintain the node state objects
 */
class NodeStateManager {
 private:
  boost::asio::io_service &_io_service;
  std::map<std::string, shared_ptr_<NodeState>> _node_states;

 public:
  explicit NodeStateManager(const App &app);

  const shared_ptr_<NodeState> &get_or_create(std::string path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
