#include "dsa/message.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(MessageOptionsTest, mergeFrom) {
  {
    SubscribeOptions initial_option;
    initial_option.priority = false;

    SubscribeOptions update_option;
    update_option.priority = true;

    EXPECT_TRUE(initial_option.mergeFrom(update_option));
  }
}
