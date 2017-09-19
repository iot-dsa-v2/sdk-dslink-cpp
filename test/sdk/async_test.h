#ifndef DSA_ASYNC_TEST_H
#define DSA_ASYNC_TEST_H

#include <functional>

#include "core/link_strand.h"

int wait_for_bool(int wait_time, const std::function<bool()>& callback);

int wait_for_bool(int wait_time, dsa::LinkStrand & strand,
                  const std::function<bool()>& callback);

#define WAIT_EXPECT_TRUE(wait_time, callback) \
  EXPECT_GT(wait_for_bool((wait_time), (callback)), -1)

#define ASYNC_EXPECT_TRUE(wait_time, strand, callback) \
  EXPECT_GT(wait_for_bool((wait_time), (strand), (callback)), -1)

#endif  // PROJECT_ASYNC_TEST_H
