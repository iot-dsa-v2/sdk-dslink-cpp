#include "dsa/util.h"

#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include "../async_test.h"
#include "core/editable_strand.h"
#include "core/shared_strand_ref.h"
using namespace dsa;

class SharedRefTestClass : public EnableRef<SharedRefTestClass> {
 public:
  int loop_count = 0;
  LinkStrandRef _strand;

  SharedRefTestClass(const LinkStrandRef& strand) : _strand(strand) {}

  void loop() {
    if (++loop_count > 1000) {
      // keep posting the loop 1000 times;
      return;
    }
    _strand->post([keep_ref = get_ref()]() { keep_ref->loop(); });
  }

  void with_parameter(string_ s) {
    get_ref();  // just mess with ref count
    loop_count += s.size();
  }
};

TEST(SharedStrandRefTest, PassingRef) {
  auto app = std::make_shared<App>();
  auto strand1 = EditableStrand::make_default(app);

  // for a new ref_ that's not used in the strand yet, you can create in other
  // thread like following code and then use post to start its job in strand
  // otherwise the shared_ptr has to be created from the strand
  auto sptr1 = SharedRef<SharedRefTestClass>::make(
      make_ref_<SharedRefTestClass>(strand1), strand1);

  POST_TO_REF(sptr1, loop /*function name*/);
  // above macro is same as:
  //  sptr1->post([](auto& t, LinkStrand&
  //  strand) {
  //    t.run();
  //  });

  string_ str = "any string";
  // with one parameter as R value
  POST_TO_REF_R(sptr1, with_parameter, str);
  // above macro is same as:
  //  sptr1->post([cv1 = std::move(str)](auto& t, LinkStrand& strand) {
  //    t.with_parameter(std::move(cv1));
  //  });

  SHARED_REF_EXPECT_TRUE(
      1000, SharedRefTestClass, sptr1,
      [](SharedRefTestClass& test) { return test.loop_count > 1000; });

  // since memory clean up need to be done in strand. shared_ref should be
  // released before the app is closed, or there will be memory leak
  sptr1.reset();

  app->close();
  app->wait();
  strand1->destroy();
}
