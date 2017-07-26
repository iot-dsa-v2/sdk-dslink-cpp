#ifndef DSA_SDK_NODE_STATE_MANAGER_H_
#define DSA_SDK_NODE_STATE_MANAGER_H_

#include <map>
#include <string>
#include <utility>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "network/app.h"
#include "node_state.h"

namespace dsa {
/**
 * maintain the node state objects
 */
class NodeStateManager {
 private:
  std::shared_ptr<App> _app;
  boost::shared_mutex _key;
  std::map<std::string, std::shared_ptr<NodeState>> _node_states;

 public:
  explicit NodeStateManager(std::shared_ptr<App> app) : _app(std::move(app)) {}

  std::shared_ptr<NodeState> get_or_create(std::string path);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_MANAGER_H_
