#include "dsa_common.h"

#include "app.h"

#include <boost/asio/strand.hpp>
#include <thread>

namespace dsa {

static void run_worker_thread(
    const shared_ptr_<boost::asio::io_service> &io_service) {
  while (true) {
    try {
      boost::system::error_code err;
      io_service->run(err);

      if (err != boost::system::errc::success) {
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

//////////////
// App
//////////////

App::App(shared_ptr_<boost::asio::io_service> io_service, size_t thread_count)
    : _io_service(std::move(io_service)),
      _threads(new boost::thread_group),
      _work(new boost::asio::io_service::work(*_io_service)) {
  // 0 thread and 1 thread are same
  if (thread_count > 1) {
    size_t hardware_concurrency = std::thread::hardware_concurrency();
    if (hardware_concurrency > 0 && thread_count > hardware_concurrency) {
      thread_count = hardware_concurrency;
    }
    for (size_t i = 0; i < thread_count; ++i)
      _threads->create_thread(boost::bind(run_worker_thread, _io_service));
  }
}
App::App(size_t thread_count)
    : App(std::make_shared<boost::asio::io_service>(thread_count),
          thread_count) {}

void App::wait() {
  if (_threads->size() == 0) {
    run_worker_thread(_io_service);
  } else {
    _threads->join_all();
  }
}

void App::sleep(unsigned int milliseconds) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

void App::close() { _work.reset(); }

boost::asio::io_service::strand *App::new_strand() {
  return new boost::asio::io_service::strand(*_io_service);
}

void App::force_stop() {
  _work.reset();
  _io_service->stop();
}

bool App::is_stopped() { return _io_service->stopped(); }

}  // namespace dsa