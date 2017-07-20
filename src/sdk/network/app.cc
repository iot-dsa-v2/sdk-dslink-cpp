#include "app.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "tcp_server.h"
#include "tcp_connection.h"

namespace dsa {

App::App(std::string name)
    : _name(name), _io_service(new boost::asio::io_service), _security_context(new SecurityContext(name + "-")) {}

App::App(std::string name, std::shared_ptr<boost::asio::io_service> io_service)
    : _name(name), _io_service(std::move(io_service)), _security_context(new SecurityContext(name + "-")) {}

void worker_thread(std::shared_ptr<boost::asio::io_service> io_service) {
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

void App::add_server(Server::Type type, Server::Config config) {
  switch (type) {
    case Server::TCP:
      _servers.push_back(std::shared_ptr<Server>(new TcpServer(*this, config)));
      return;
    default:
      throw std::runtime_error("invalid server type");
  }
}

ClientPtr App::new_client(Client::Config config) {
  return std::shared_ptr<Connection>(new TcpClientConnection(*this, config));
}

void App::run(unsigned int thread_count) {
  if (thread_count == 0u) return;

  boost::asio::io_service::work work(*_io_service);

  boost::thread_group threads;
  for (size_t i = 0; i < thread_count; ++i)
    threads.create_thread(boost::bind(worker_thread, _io_service));

  // start servers
  for (ServerPtr server : _servers)
    server->start();

  threads.join_all();
}

void App::async_run(unsigned int thread_count) {
  boost::thread_group threads;
  threads.create_thread(boost::bind(&App::run, this, thread_count));
}

}  // namespace dsa