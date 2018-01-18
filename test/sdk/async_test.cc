#include "dsa_common.h"

#include "async_test.h"

#include <boost/asio/strand.hpp>

int wait_for_bool(int wait_time, const std::function<bool()>& callback) {
  int waited = 0;
  while (waited < wait_time) {
    if (callback()) {
      return waited;
    }
    boost::this_thread::sleep(
        boost::posix_time::milliseconds(WAIT_SLEEP_INTERVAL));
    waited += WAIT_SLEEP_INTERVAL;
  }
  return -1;
}

int wait_for_bool(int wait_time, dsa::LinkStrand& strand,
                  const std::function<bool()>& callback) {
  bool succeed = false;
  int waited = 0;
  while (waited < wait_time) {
    strand.dispatch([&, callback]() {
      if (callback()) {
        succeed = true;
      }
    });
    boost::this_thread::sleep(
        boost::posix_time::milliseconds(WAIT_SLEEP_INTERVAL));
    waited += WAIT_SLEEP_INTERVAL;
    if (succeed) {
      return waited;
    }
  }
  if (succeed) {
    return waited;
  }
  return -1;
}

void wait_in_thread(int wait_time) {
  boost::this_thread::sleep(boost::posix_time::milliseconds(wait_time));
}
