#include "dsa_common.h"

#include "app.h"


#include <boost/thread.hpp>

namespace dsa {
//////////////
// App
//////////////
App::App(std::string name)
    : _name(name),
      _io_service(new boost::asio::io_service),
      _threads(new boost::thread_group) {}

App::App(std::string name, shared_ptr_<boost::asio::io_service> io_service)
    : _name(name),
      _io_service(std::move(io_service)),
      _threads(new boost::thread_group) {}

void run_worker_thread(const shared_ptr_<boost::asio::io_service> &io_service) {
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

void App::async_start(unsigned int thread_count) {
  if (thread_count == 0u) return;

  _work.reset(new boost::asio::io_service::work(*_io_service));

  for (size_t i = 0; i < thread_count; ++i)
    _threads->create_thread(boost::bind(run_worker_thread, _io_service));
}

void App::wait() { _threads->join_all(); }

void App::run(unsigned int thread_count) {
  async_start(thread_count);
  wait();
}

void App::sleep(unsigned int milliseconds) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

void App::close() { _work.reset(); }

boost::asio::io_service::strand *App::new_strand() {
  return new boost::asio::io_service::strand(*_io_service);
}

void App::force_stop() { _work.reset(); }

}  // namespace dsa