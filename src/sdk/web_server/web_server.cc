#include "dsa_common.h"

#include "web_server.h"

#include <algorithm>
#include <thread>
#include <vector>

namespace dsa {

WebServer::WebServer()
    :
      _thread_count(std::min(
          std::max<std::size_t>(1, std::thread::hardware_concurrency()),
          thread_count)) {}

void WebServer::start() {
  //boost::asio::io_service ios{_thread_count};

  // create and launch listener

  std::vector<std::thread> v;
  v.reserve(_thread_count - 1);
  for (auto i = _thread_count - 1; i > 0; --i)
    v.emplace_back([&ios] { ios.run(); });
  ios.run();
}

void WebServer::destroy() {}

WebServer::~WebServer() {}

}  // namespace dsa
