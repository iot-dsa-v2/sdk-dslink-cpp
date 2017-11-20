#include "dsa/network.h"

#include "../test_config.h"
#include "gtest/gtest.h"

using namespace dsa;


TEST(PlayTest, SESSION) {
    App app;

    // WrapperConfig config = TestConfig(app);
    // config.destroy();

//    auto config = make_ref_<EditableStrand>(app.new_strand(), make_unique_<ECDH>());
//
//    Session s(config, "hello");                    // invalid throws exception at the end of scope
//    s.destroy();
//
////    Session* s2 = new Session(config, "hello");    // invalid throws exception on delete it
////    delete s2;
////
//    auto s3 = make_ref_<Session>(config, "hello");
//    s3.get()->destroy();
//
//    // MEMLEAK ????

}