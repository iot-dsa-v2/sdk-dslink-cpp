#include "dsa/util.h"
#include "gtest/gtest.h"

using namespace dsa;

TEST(PathTest, Path__invalid_name) {}

TEST(PathTest, Path__is_invalid) {
  {
    Path p("//");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/$/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/@/");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p(".");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/./to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/../to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/.");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/..");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/\to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/\\to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/\"to");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/%");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/?");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/*");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/:");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/>");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("/path/<");
    EXPECT_TRUE(p.is_invalid());
  }
  {
    Path p("<");
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
    Path p("/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/");
    EXPECT_TRUE(p.is_invalid());
  }
}

TEST(PathTest, Path__is_root) {
  {
    Path p("");

    EXPECT_TRUE(p.is_root());
  }
  {
    Path p("/");
    EXPECT_TRUE(p.is_root());
  }
}

TEST(PathTest, Path__is_node) {
  {
    Path p("/path/to/dsa$conf");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/path/to/dsa@conf");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/a/b/path.");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/a/b/path..");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/a/b/path.name");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/a/b/path..name");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/path/ ");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/path/ /to");
    EXPECT_TRUE(p.is_node());
  }
  {
    Path p("/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789/0123456789");
    EXPECT_TRUE(p.is_node());
  }
}

TEST(PathTest, Path__is_config) {
  {
    Path p("$");
    EXPECT_TRUE(p.is_config());
  }
  {
    Path p("/$");
    EXPECT_TRUE(p.is_config());
  }
  {
    Path p("/$$");
    EXPECT_TRUE(p.is_config());
  }
  {
    Path p("/$c$o$n$f$i$g$");
    EXPECT_TRUE(p.is_config());
  }
  {
    Path p("$config");
    EXPECT_TRUE(p.is_config());
  }
  {
    Path p("/path/to/dsa/$config");
    EXPECT_TRUE(p.is_config());
  }
}

TEST(PathTest, Path__is_attribute) {
  {
    Path p("@");
    EXPECT_TRUE(p.is_attribute());
  }
  {
    Path p("/@");
    EXPECT_TRUE(p.is_attribute());
  }
  {
    Path p("/@@");
    EXPECT_TRUE(p.is_attribute());
  }
  {
    Path p("/@a@t@t@r");
    EXPECT_TRUE(p.is_attribute());
  }
  {
    Path p("@attr");
    EXPECT_TRUE(p.is_attribute());
  }
  {
    Path p("/path/to/dsa/@config");
    EXPECT_TRUE(p.is_attribute());
  }
}

TEST(PathTest, Path__current__next__last) {
  Path path("/path/to/dsa/@config");

  EXPECT_EQ("path", path.current());

  auto p1 = path.next();
  EXPECT_EQ("to", p1.current());

  auto p2 = p1.next();
  EXPECT_EQ("dsa", p2.current());

  EXPECT_TRUE(p2.is_last_node());

  auto p3 = p2.next();
  EXPECT_EQ("@config", p3.current());

  EXPECT_TRUE(p3.is_last());
}

TEST(PathTest, Path__copy) {
  {
    Path src_path("p1/p2/p3/@attr");
    Path des_path = src_path.copy();

    EXPECT_TRUE(des_path.is_attribute());
    EXPECT_EQ(des_path.data->str, src_path.data->str);
    EXPECT_EQ(des_path.current(), src_path.current());
    EXPECT_NE(des_path.data.get(), src_path.data.get());
  }

  {
    Path src_path("//");
    Path des_pat = src_path.copy();
  }
}
