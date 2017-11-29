#include "dsa/util.h"
#include "gtest/gtest.h"
#include "../util/broker_runner.h"
using namespace dsa;

TEST(DateTime, DateTime) {
  auto broker = create_broker();
  broker->run();
}
