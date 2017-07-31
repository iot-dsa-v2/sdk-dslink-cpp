#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include <map>
#include <string>
#include <utility>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/io_service.hpp>

#include "network/app.h"
#include "node_state.h"

namespace dsa {
/**
 * maintain the node state objects
 */
class NodeStateManager {
 private:
  boost::asio::io_service &_io_service;
  boost::shared_mutex _key;
  std::map<std::string, std::shared_ptr<NodeState>> _node_states;

 public:
  explicit NodeStateManager(const App &app) : _io_service(app.io_service()) {}

  std::shared_ptr<NodeState> get_or_create(std::string path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
