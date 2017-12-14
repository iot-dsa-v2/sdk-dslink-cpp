#include "dsa/crypto.h"
#include "gtest/gtest.h"

#include <thread>
#include <boost/format.hpp>
#include <module/logger.h>
#include <boost/filesystem/operations.hpp>
#include <util/string.h>
#include "dsa/responder.h"
#include "dslink.h"

using boost::format;

using namespace dsa;
using namespace std;

#define DEFAULT_DS_PORT 4120
#define DEFAULT_DSS_PORT 4128
#define DEFAULT_WS_PORT 80
#define DEFAULT_WSS_PORT 443
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_THREAD 0
#define DEFAULT_TCP_SERVER_PORT 0

#define CREATE_TEST_DSLINK auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");

TEST(DSLinkTest, default_param) {

  const char *argv[] = {"./test"};
  int argc = 1;
  CREATE_TEST_DSLINK;

  EXPECT_STREQ(DEFAULT_HOST, link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  EXPECT_EQ(DEFAULT_THREAD, link.get()->get_app().get_thread_size());
  EXPECT_STREQ("mydslink", link.get()->dsid_prefix.c_str());
  EXPECT_EQ(DEFAULT_TCP_SERVER_PORT, link.get()->tcp_server_port);
  EXPECT_STREQ("", link.get()->ws_host.c_str());
  EXPECT_EQ(0, link.get()->ws_port);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param1) {

  const char *argv[] = {"./test", "-b", "192.168.1.12"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param2) {

  const char *argv[] = {"./test", "--broker", "192.168.1.12"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param3) {

  const char *argv[] = {"./test", "--broker", "dss://192.168.1.12"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DSS_PORT, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param4) {

  const char *argv[] = {"./test", "--broker", "dss://192.168.1.12:132"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(132, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param5) {

  const char *argv[] = {"./test", "--broker", "ws://192.168.1.12"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(DEFAULT_WS_PORT, link.get()->ws_port);
  EXPECT_FALSE(link.get()->secure);
  EXPECT_STREQ("", link.get()->tcp_host.c_str());
  EXPECT_EQ(0, link.get()->tcp_port);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param6) {

  const char *argv[] = {"./test", "--broker", "wss://192.168.1.12"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(DEFAULT_WSS_PORT, link.get()->ws_port);
  EXPECT_TRUE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, url_param7) {

  const char *argv[] = {"./test", "--broker", "wss://192.168.1.12:132"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(132, link.get()->ws_port);
  EXPECT_TRUE(link.get()->secure);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param1) {

  const char *argv[] = {"./test", "-l", "invalid_val"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param2) {

  const char *argv[] = {"./test", "-l", "all"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::ALL___, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param3) {

  const char *argv[] = {"./test", "-l", "trace"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::TRACE_, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param4) {

  const char *argv[] = {"./test", "-l", "debug"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::DEBUG_, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param5) {

  const char *argv[] = {"./test", "-l", "error"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::ERROR_, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param6) {

  const char *argv[] = {"./test", "-l", "warn"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::WARN__, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param7) {

  const char *argv[] = {"./test", "-l", "fatal"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::FATAL_, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, log_param8) {

  const char *argv[] = {"./test", "-l", "none"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(Logger::NONE__, link.get()->strand.get()->logger().level);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, thread_param1) {

  const char *argv[] = {"./test", "--thread", "0"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(0, link.get()->get_app().get_thread_size());
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, thread_param2) {

  const char *argv[] = {"./test", "--thread", "1"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(0, link.get()->get_app().get_thread_size());
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, thread_param3) {

  const char *argv[] = {"./test", "--thread", "2"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(2, link.get()->get_app().get_thread_size());
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, thread_param4) {

  const char *argv[] = {"./test", "--thread", "20"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_LE(link.get()->get_app().get_thread_size(), 16);
  EXPECT_LE(link.get()->get_app().get_thread_size(), std::thread::hardware_concurrency());
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, tcp_server_port_param) {

  const char *argv[] = {"./test", "--server-port", "132"};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(link.get()->tcp_server_port, 132);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, token_file) {

  string_ token("IAmATokenPleaseBelieveME!!!");

  // First create token file
  string_ token_file_name("my_test_token.txt");
  if(!boost::filesystem::exists(token_file_name)){
    string_to_file(token, token_file_name);
  }

  const char *argv[] = {"./test", "--token", token_file_name.c_str()};
  int argc = 3;
  CREATE_TEST_DSLINK

  EXPECT_EQ(link.get()->client_token, token);
  link.get()->destroy();
  link.reset();
}

TEST(DSLinkTest, general_param) {

  const char
      *argv[] = {"./test", "--broker", "wss://192.168.1.12:142", "-l", "info", "--thread", "2", "--server-port", "132"};
  int argc = 9;
  CREATE_TEST_DSLINK

  EXPECT_STREQ("", link.get()->tcp_host.c_str());
  EXPECT_EQ(0, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  EXPECT_EQ(2, link.get()->get_app().get_thread_size());
  EXPECT_STREQ("mydslink", link.get()->dsid_prefix.c_str());
  EXPECT_EQ(132, link.get()->tcp_server_port);
  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(142, link.get()->ws_port);
  link.get()->destroy();
  link.reset();
}
