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
  explicit App(size_t thread_count = 4);
  App(shared_ptr_<boost::asio::io_service> io_service,
      size_t thread_count = 4);
  boost::asio::io_service &io_service() const { return *_io_service; };


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
