#include "dsa/network.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(AppTest, NewApp) {
  std::shared_ptr<App> app;
  ASSERT_NO_FATAL_FAILURE(app.reset(new App("Test")));
//  ASSERT_NO_FATAL_FAILURE(app->run());
}
