//#include <iostream>

#include "dsa/util.h"
#include "gtest/gtest.h"

class RefCheck : public dsa::EnableShared<RefCheck> {
 public:
  static int count;
  RefCheck(int a) { ++count; }
  ~RefCheck() { --count; }
};
int RefCheck::count = 0;

TEST(EnableSharedTest, RefCheck) {
  std::shared_ptr<RefCheck> ptr = dsa::make_shared<RefCheck>(1);
  EXPECT_EQ(RefCheck::count, 1);
  ptr->destroy();
  EXPECT_EQ(RefCheck::count, 1);
  ptr.reset();
  EXPECT_EQ(RefCheck::count, 0);
}
