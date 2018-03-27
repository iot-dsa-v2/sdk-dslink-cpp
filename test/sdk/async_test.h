#ifndef DSA_ASYNC_TEST_H
#define DSA_ASYNC_TEST_H

#include <algorithm>
#include <boost/thread.hpp>
#include <functional>
#include "core/link_strand.h"
#include "core/shared_strand_ref.h"

namespace dsa {
class LinkStrand;
}

const int WAIT_SLEEP_INTERVAL = 10;

int wait_for_bool(int wait_time, const std::function<bool()>& callback);

int wait_for_bool(int wait_time, dsa::LinkStrand& strand,
                  const std::function<bool()>& callback);

template <class T>
int wait_for_bool(int wait_time, std::shared_ptr<dsa::SharedRef<T>>& sptr,
                  const std::function<bool(T&)>& callback) {
  bool succeed = false;
  int waited = 0;
  while (waited < wait_time) {
    sptr->post([&, callback](dsa::ref_<T>& t, dsa::LinkStrand& strand) {
      if (callback(*t)) {
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

void wait_in_thread(int wait_time);

#define WAIT_EXPECT_TRUE(wait_time, callback) \
  EXPECT_TRUE(wait_for_bool((wait_time), (callback)) > -1)

#define ASYNC_EXPECT_TRUE(wait_time, strand, callback) \
  EXPECT_TRUE(wait_for_bool((wait_time), (strand), (callback)) > -1)

#define SHARED_REF_EXPECT_TRUE( wait_time, CLASS,strand, callback) \
  EXPECT_TRUE(wait_for_bool<CLASS>((wait_time), (strand), (callback)) > -1)

// Added for syntax coloring, it helps to see test steps
#define WAIT(wait_time) wait_in_thread(wait_time);

#define EXPECT_CONTAIN(iteratable, value)                               \
  EXPECT_TRUE(std::find(iteratable.begin(), iteratable.end(), value) != \
              iteratable.end());

#define EXPECT_NOT_CONTAIN(iteratable, value)                           \
  EXPECT_TRUE(std::find(iteratable.begin(), iteratable.end(), value) == \
              iteratable.end());

#endif  // PROJECT_ASYNC_TEST_H
