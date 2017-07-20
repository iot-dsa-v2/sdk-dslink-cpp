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

App::App(std::string name)
    : _name(name), _io_service(new boost::asio::io_service), _security_context(new SecurityContext(name + "-")),
      _threads(new boost::thread_group) {}

App::App(std::string name, std::shared_ptr<boost::asio::io_service> io_service)
    : _name(name), _io_service(std::move(io_service)), _security_context(new SecurityContext(name + "-")),
      _threads(new boost::thread_group) {}

void worker_thread(const std::shared_ptr<boost::asio::io_service> &io_service) {
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

ServerPtr App::new_server(Server::Type type, const Server::Config &config) {
  switch (type) {
    case Server::TCP:
      return std::shared_ptr<Server>(new TcpServer(*this, config));
    default:
      throw std::runtime_error("invalid server type");
  }
}

ClientPtr App::new_client(Client::Type type, const Client::Config &config) {
  switch (type) {
    case Client::TCP:
      return std::shared_ptr<Connection>(new TcpClientConnection(*this, config));
    default:
      throw std::runtime_error("invalid client type");
  }
}

void App::async_start(unsigned int thread_count) {
  if (thread_count == 0u) return;

  _work.reset(new io_service_work(*_io_service));

  for (size_t i = 0; i < thread_count; ++i)
    _threads->create_thread(boost::bind(worker_thread, _io_service));
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
}

void App::stop() {
  _io_service->stop();
}

}  // namespace dsa