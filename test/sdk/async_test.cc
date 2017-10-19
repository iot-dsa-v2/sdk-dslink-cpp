#include "dsa_common.h"

#include "async_test.h"

#include <atomic>
#include <boost/asio/strand.hpp>
#include <boost/thread.hpp>

const int SLEEP_INTERVAL = 10;

int wait_for_bool(int wait_time, const std::function<bool()>& callback) {
  int waited = 0;
  while (waited < wait_time) {
    if (callback()) {
      return waited;
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_INTERVAL));
    waited += SLEEP_INTERVAL;
  }
  return -1;
}

int wait_for_bool(int wait_time, dsa::LinkStrand& strand,
                  const std::function<bool()>& callback) {
  std::atomic_bool succeed{false};
  int waited = 0;
  while (waited < wait_time) {
    strand.dispatch([&, callback]() {
      if (callback()) {
        succeed = true;
      }
    });
    boost::this_thread::sleep(boost::posix_time::milliseconds(SLEEP_INTERVAL));
    waited += SLEEP_INTERVAL;
    if (succeed) {
      return waited;
    }
  }
  if (succeed) {
    return waited;
  }
  return -1;
}