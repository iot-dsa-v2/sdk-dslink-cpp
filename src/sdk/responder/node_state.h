#ifndef DSA_SDK_NODE_STATE_H_
#define DSA_SDK_NODE_STATE_H_

#include <string>
#include <vector>

#include <boost/asio/strand.hpp>

#include "network/session.h"

namespace dsa {
/**
 * maintain streams of a node
 */
class NodeState {
 private:
  boost::asio::io_service::strand _strand;
  std::string _path;
  std::vector<std::shared_ptr<Session>> _sessions;

 public:
  explicit NodeState(boost::asio::io_service &io_service, std::string path);

  //////////////////////////
  // Getters
  //////////////////////////
  const std::string &path() { return _path; }

  //////////////////////////
  // Setters
  //////////////////////////
  void add_session(std::shared_ptr<Session> session);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_STATE_H_
