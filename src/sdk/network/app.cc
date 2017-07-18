//
// Created by Ben Richards on 7/17/17.
//

#include "app.h"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "security_context.h"

namespace dsa {

App::App(std::string name)
    : _name(name), _io_service(new boost::asio::io_service), _security_context(new SecurityContext(name + "-")) {}

App::App(std::string name, std::shared_ptr<boost::asio::io_service> io_service)
    : _name(name), _io_service(io_service), _security_context(new SecurityContext(name + "-")) {}

boost::asio::io_service &App::io_service() {
  return *_io_service;
}

SecurityContext &App::security_context() {
  return *_security_context;
}

std::string App::name() const {
  return _name;
}

void worker_thread(std::shared_ptr<boost::asio::io_service> io_service) {
  while (true) {
    try {
      boost::system::error_code err;
      io_service->run(err);

      if (err) {
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

void App::run(unsigned int thread_count) {
  if (!thread_count) return;

  boost::asio::io_service::work work(*_io_service);

  boost::thread_group threads;
  for (size_t i = 0; i < thread_count; ++i)
    threads.create_thread(boost::bind(worker_thread, _io_service));

  threads.join_all();
}

}  // namespace dsa