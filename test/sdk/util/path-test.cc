#include "dsa/util.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(PathTest, Path__invalid_name) {}

TEST(PathTest, Path__is_invalid) {
  {
    Path p("/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("a/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/a");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("$/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("@/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p(".");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/./to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/../to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/.");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/..");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/\to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/\\to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/\"to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/%");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/?");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("path/*");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("</");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    char data[] = {0x1f};
    Path p(data);
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("%0");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("%%00");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("%%0z");
    EXPECT_TRUE(p.is_invalid());
  }
}

TEST(PathTest, Path__is_root) {
  {
    Path p("");

    EXPECT_TRUE(p.is_root());
  }

}

TEST(PathTest, Path__is_node) {
  {
    Path p("path/to/dsa$conf");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("path/to/dsa@conf");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("a/b/path.");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("a/b/path..");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("a/b/path.name");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("a/b/path..name");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("path/ ");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("path/ /to");
    EXPECT_FALSE(p.is_invalid());
  }
  {
    Path p("0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789");
    EXPECT_FALSE(p.is_invalid());
  }
}

//TEST(PathTest, Path__is_metadata) {
//  {
//    Path p("$");
//    EXPECT_TRUE(p.is_metadata());
//  }
//  {
//    Path p("$$");
//    EXPECT_TRUE(p.is_metadata());
//  }
//  {
//    Path p("$c$o$n$f$i$g$");
//    EXPECT_TRUE(p.is_metadata());
//  }
//  {
//    Path p("$metadata");
//    EXPECT_TRUE(p.is_metadata());
//  }
//  {
//    Path p("path/to/dsa/$metadata");
//    EXPECT_TRUE(p.is_metadata());
//  }
//}

//TEST(PathTest, Path__is_attribute) {
//  {
//    Path p("@");
//    EXPECT_TRUE(p.is_attribute());
//  }
//
//  {
//    Path p("@@");
//    EXPECT_TRUE(p.is_attribute());
//  }
//  {
//    Path p("@a@t@t@r");
//    EXPECT_TRUE(p.is_attribute());
//  }
//  {
//    Path p("@attr");
//    EXPECT_TRUE(p.is_attribute());
//  }
//  {
//    Path p("path/to/dsa/@att");
//    EXPECT_TRUE(p.is_attribute());
//  }
//}

TEST(PathTest, Path__current__next__last) {
  Path path("path/to/dsa");

  EXPECT_EQ("path", path.current_name());

  auto p1 = path.next();
  EXPECT_EQ("to", p1.current_name());

  auto p2 = p1.next();
  EXPECT_EQ("dsa", p2.current_name());

  EXPECT_TRUE(p2.is_last());
  
}

TEST(PathTest, Path__copy) {
  {
    Path src_path("p1/p2/p3");
    Path des_path = src_path.copy();

    EXPECT_FALSE(des_path.is_invalid());
    EXPECT_EQ(des_path.full_str(), src_path.full_str());
    EXPECT_EQ(des_path.current_name(), src_path.current_name());
    EXPECT_NE(des_path.data().get(), src_path.data().get());
  }

  {
    Path src_path("//");
    Path des_pat = src_path.copy();
  }
}
