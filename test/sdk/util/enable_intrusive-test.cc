#include "dsa/util.h"
#include "gtest/gtest.h"

namespace {
class RefCheck : public dsa::EnableIntrusive<RefCheck> {
 public:
  static int count;
  RefCheck(int a) { ++count; }
  ~RefCheck() { --count; }
  dsa::intrusive_ptr_<RefCheck> get_intrusive_from_this() {
    return intrusive_this();
  }
};
int RefCheck::count = 0;
}

TEST(EnableIntrusiveTest, RefCheck) {
  auto ptr = dsa::make_intrusive_<RefCheck>(1);
  EXPECT_EQ(ptr->ref_count(), 1);
  auto this_ptr = ptr->get_intrusive_from_this();
  EXPECT_EQ(ptr->ref_count(), 2);
  this_ptr.reset();
  EXPECT_EQ(ptr->ref_count(), 1);
  ptr.reset();
}
