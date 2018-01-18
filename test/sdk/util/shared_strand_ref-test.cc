#include "dsa/util.h"

#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include "core/editable_strand.h"
#include "core/shared_strand_ref.h"

using namespace dsa;

class SharedRefTestClass : public EnableRef<SharedRefTestClass> {
 public:
  int loop = 0;
  SharedRefTestClass() {}

  void run(LinkStrandRef strand) {
    if (++loop > 1000) {
      // keep posting the loop 1000 times;
      return;
    }
    auto p_strand = strand.get();
    p_strand->post([ keep_ref = get_ref(), strand = std::move(strand) ]() {
      keep_ref->run(std::move(strand));
    });
  }
};

TEST(SharedStrandRefTest, PassingRef) {
  auto app = std::make_shared<App>();
  auto strand1 = EditableStrand::make_default(app);

  // if the ref being wrapped by this shared_ptr is already used in strand, then
  // the shared_ptr has to be created from the strand
  // for a new ref_ that's not used in the strand yet, you can create in other
  // thread like following code and use post to start its job in strand
  auto sptr1 = SharedRef<SharedRefTestClass>::make(
      make_ref_<SharedRefTestClass>(), strand1);

  POST_TO_REF_1(sptr1, run /*function name*/, strand1->get_ref() /*parameter*/);
  // above macro is same as:
  //  sptr1->post([s = std::move(strand1->get_ref())](auto& t, LinkStrand&
  //  strand) {
  //    t.run(std::move(s));
  //  });

  bool done = false;
  while (!done) {
    sptr1->post([&done](auto& test, auto& strand) {
      if (test.loop > 1000) {
        done = true;
      }
    });
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }
  // since memory clean up need to be done in strand. shared_ref should be
  // released before the app is closed, or there will be memory leak

  sptr1.reset();

  app->close();
  app->wait();
  strand1->destroy();
}
