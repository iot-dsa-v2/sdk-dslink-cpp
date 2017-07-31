#include "dsa_common.h"

#include "app.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tcp_server.h"
#include "tcp_connection.h"

namespace dsa {
class io_service_work : public boost::asio::io_service::work {
 public:
  explicit io_service_work(boost::asio::io_service &io_service) : boost::asio::io_service::work(io_service) {}
};

//////////////
// App
//////////////
App::App(std::string name)
    : _name(name), _io_service(new boost::asio::io_service), _security_context(new SecurityContext(name + "-")),
      _threads(new boost::thread_group) {}

App::App(std::string name, std::shared_ptr<boost::asio::io_service> io_service)
    : _name(name), _io_service(std::move(io_service)), _security_context(new SecurityContext(name + "-")),
      _threads(new boost::thread_group) {}

void run_worker_thread(const std::shared_ptr<boost::asio::io_service> &io_service) {
  while (true) {
    try {
      boost::system::error_code err;
      io_service->run(err);

      if (err != nullptr) {
        // TODO: log error message to file?
      } else {
        return;
      }
    } catch (std::exception &e) {
      // TODO: log error message to file?
      return;
    }
  }
}

Server *App::new_server(Server::Protocol type, const Server::Config &config) throw() {
  switch (type) {
    case Server::TCP:return new TcpServer(shared_from_this(), config);
    default:throw std::runtime_error("invalid server type");
  }
}

Client *App::new_client(Client::Protocol type, const Client::Config &config) throw() {
  switch (type) {
    case Client::TCP:return new TcpClientConnection(shared_from_this(), config);
    default:throw std::runtime_error("invalid client type");
  }
}

void App::async_start(unsigned int thread_count) {
  if (thread_count == 0u) return;

  _work.reset(new io_service_work(*_io_service));

  for (size_t i = 0; i < thread_count; ++i)
    _threads->create_thread(boost::bind(run_worker_thread, _io_service));
}

void App::wait() {
  _threads->join_all();
}

void App::run(unsigned int thread_count) {
  async_start(thread_count);
  wait();
}

void App::sleep(unsigned int milliseconds) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

void App::graceful_stop() {
  _work.reset();

  // tell all registered components to shutdown
  for (auto &kv : _registry)
    if (auto component = kv.second.lock())
      component->stop();
}

void App::graceful_stop(unsigned int milliseconds) {
  graceful_stop();
  sleep(milliseconds);
  stop();
}

void App::stop() {
  _io_service->stop();
}

void App::register_component(std::shared_ptr<GracefullyClosable> component) {
  std::lock_guard<std::mutex> lock(_register_key);
  _registry[component.get()] = std::move(component);
}

void App::unregister_component(void *component) {
  std::lock_guard<std::mutex> lock(_register_key);
  _registry.erase(component);
}

}  // namespace dsa