#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>

#include "util/enable_shared.h"

namespace dsa {

class App {
 private:
  shared_ptr_<boost::asio::io_service> _io_service;
  shared_ptr_<boost::asio::io_service::work> _work;
  shared_ptr_<boost::thread_group> _threads;

 public:
  explicit App();
  App(shared_ptr_<boost::asio::io_service> io_service);
  boost::asio::io_service &io_service() const { return *_io_service; };

  // start io_service and wait forever
  void run(unsigned int thread_count = 5);

  // start io_service asynchronously and continue
  void async_start(unsigned int thread_count = 5);

  // allows jobs to finish then stops io_service, may not stop if servers or
  // clients are listening
  void close();

  // halts jobs and stops io_service
  void force_stop();

  bool is_stopped();

  // wait forever or until all worker threads fail
  void wait();

  // sleep current thread in milliseconds
  void sleep(unsigned int milliseconds);

  boost::asio::io_service::strand *new_strand();
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_APP_H_
