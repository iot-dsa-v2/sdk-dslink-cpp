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
  auto ptr = dsa::make_shared<RefCheck>(1);
  EXPECT_EQ(RefCheck::count, 1);
  ptr->destroy();
  EXPECT_EQ(RefCheck::count, 1);
  ptr.reset();
  EXPECT_EQ(RefCheck::count, 0);
}
