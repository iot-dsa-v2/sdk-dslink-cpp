#ifndef DSA_ASYNC_TEST_H
#define DSA_ASYNC_TEST_H

#include <functional>

#include <boost/asio/io_service.hpp>

bool wait_for_bool(int wait_time, const std::function<bool()>& callback);

bool wait_for_bool(int wait_time, boost::asio::io_service::strand* strand,
                   const std::function<bool()>& callback);

#define WAIT_EXPECT_TRUE(wait_time, callback) \
  EXPECT_TRUE(wait_for_bool((wait_time), (callback)))

#define ASYNC_EXPECT_TRUE(wait_time, strand, callback) \
  EXPECT_TRUE(wait_for_bool((wait_time), (strand), (callback)))

#endif  // PROJECT_ASYNC_TEST_H
