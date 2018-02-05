#include "dsa/crypto.h"
#include "dsa/responder.h"

#include <gtest/gtest.h>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <thread>
#include "../test_config.h"
#include "dslink.h"
#include "module/default/console_logger.h"
#include "module/logger.h"
#include "util/string.h"

using boost::format;

using namespace dsa;
using namespace std;

using DslinkTest = SetUpBase;

#define DEFAULT_DS_PORT 4120
#define DEFAULT_DSS_PORT 4128
#define DEFAULT_WS_PORT 80
#define DEFAULT_WSS_PORT 443
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_THREAD 0
#define DEFAULT_TCP_SERVER_PORT 0

static ref_<DsLink> create_test_dslink(int argc, const char *argv[]) {
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0");
  // filter log for unit test
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  return std::move(link);
}

void end_link(ref_<DsLink> &&link) {
  link.get()->destroy();
  link->get_app().wait();
  link.reset();
}

TEST_F(DslinkTest, DefaultParam) {
  const char *argv[] = {"./test"};
  int argc = 1;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ(DEFAULT_HOST, link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  EXPECT_EQ(DEFAULT_THREAD, link.get()->get_app().get_thread_size());
  EXPECT_STREQ("mydslink", link.get()->dsid_prefix.c_str());
  EXPECT_EQ(DEFAULT_TCP_SERVER_PORT, link.get()->tcp_server_port);
  EXPECT_STREQ("", link.get()->ws_host.c_str());
  EXPECT_EQ(0, link.get()->ws_port);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam1) {
  const char *argv[] = {"./test", "-b", "192.168.1.12"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam2) {
  const char *argv[] = {"./test", "--broker", "192.168.1.12"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DS_PORT, link.get()->tcp_port);
  EXPECT_FALSE(link.get()->secure);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam3) {
  const char *argv[] = {"./test", "--broker", "dss://192.168.1.12"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(DEFAULT_DSS_PORT, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  end_link(std::move(link));
  ;
}

TEST_F(DslinkTest, url_param4) {
  const char *argv[] = {"./test", "--broker", "dss://192.168.1.12:132"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->tcp_host.c_str());
  EXPECT_EQ(132, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam5) {
  const char *argv[] = {"./test", "--broker", "ws://192.168.1.12"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(DEFAULT_WS_PORT, link.get()->ws_port);
  EXPECT_FALSE(link.get()->secure);
  EXPECT_STREQ("", link.get()->tcp_host.c_str());
  EXPECT_EQ(0, link.get()->tcp_port);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam6) {
  const char *argv[] = {"./test", "--broker", "wss://192.168.1.12"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(DEFAULT_WSS_PORT, link.get()->ws_port);
  EXPECT_TRUE(link.get()->secure);
  end_link(std::move(link));
}

TEST_F(DslinkTest, UrlParam7) {
  const char *argv[] = {"./test", "--broker", "wss://192.168.1.12:132"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(132, link.get()->ws_port);
  EXPECT_TRUE(link.get()->secure);
  end_link(std::move(link));
}

TEST_F(DslinkTest, LogParam1) {
  const char *argv[] = {"./test", "-l", "invalid_val"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

//// comment out this test to avoid unecessary console output
// TEST(DslinkTest, log_param2) {
//  const char *argv[] = {"./test", "-l", "all"};
//  int argc = 3;
//  auto link = create_test_dslink(argc, argv);
//
//  EXPECT_EQ(Logger::ALL___, link.get()->strand.get()->logger().level);
//  end_link(std::move(link));
//}
//// comment out this test to avoid unecessary console output
// TEST(DslinkTest, log_param3) {
//  const char *argv[] = {"./test", "-l", "trace"};
//  int argc = 3;
//  auto link = create_test_dslink(argc, argv);
//
//  EXPECT_EQ(Logger::TRACE_, link.get()->strand.get()->logger().level);
//  end_link(std::move(link));
//}

TEST_F(DslinkTest, LogParam4) {
  const char *argv[] = {"./test", "-l", "debug"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::DEBUG_, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

TEST_F(DslinkTest, LogParam5) {
  const char *argv[] = {"./test", "-l", "error"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::ERROR_, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

TEST_F(DslinkTest, LogParam6) {
  const char *argv[] = {"./test", "-l", "warn"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::WARN__, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

TEST_F(DslinkTest, LogParam7) {
  const char *argv[] = {"./test", "-l", "fatal"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::FATAL_, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

TEST_F(DslinkTest, LogParam8) {
  const char *argv[] = {"./test", "-l", "none"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(Logger::NONE__, link.get()->strand.get()->logger().level);
  end_link(std::move(link));
}

TEST_F(DslinkTest, ThreadParam1) {
  const char *argv[] = {"./test", "--thread", "0"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(0, link.get()->get_app().get_thread_size());
  end_link(std::move(link));
}

TEST_F(DslinkTest, ThreadParam2) {
  const char *argv[] = {"./test", "--thread", "1"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(0, link.get()->get_app().get_thread_size());
  end_link(std::move(link));
}

TEST_F(DslinkTest, ThreadParam3) {
  const char *argv[] = {"./test", "--thread", "2"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(2, link.get()->get_app().get_thread_size());
  end_link(std::move(link));
}

TEST_F(DslinkTest, ThreadParam4) {
  const char *argv[] = {"./test", "--thread", "20"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_LE(link.get()->get_app().get_thread_size(), 16);
  EXPECT_LE(link.get()->get_app().get_thread_size(),
            std::thread::hardware_concurrency());
  end_link(std::move(link));
}

TEST_F(DslinkTest, TCPServerPortParam) {
  const char *argv[] = {"./test", "--server-port", "132"};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(link.get()->tcp_server_port, 132);
  end_link(std::move(link));
}

TEST_F(DslinkTest, TokenFile) {
  string_ token("IAmATokenPleaseBelieveME!!!");

  // First create token file
  string_ token_file_name("my_test_token.txt");
  if (!boost::filesystem::exists(token_file_name)) {
    string_to_file(token, token_file_name);
  }

  const char *argv[] = {"./test", "--token", token_file_name.c_str()};
  int argc = 3;
  auto link = create_test_dslink(argc, argv);

  EXPECT_EQ(link.get()->client_token, token);
  end_link(std::move(link));
}

TEST_F(DslinkTest, GeneralParam) {
  const char *argv[] = {"./test", "--broker",      "wss://192.168.1.12:142",
                        "-l",     "info",          "--thread",
                        "2",      "--server-port", "132"};
  int argc = 9;
  auto link = create_test_dslink(argc, argv);

  EXPECT_STREQ("", link.get()->tcp_host.c_str());
  EXPECT_EQ(0, link.get()->tcp_port);
  EXPECT_TRUE(link.get()->secure);
  EXPECT_EQ(Logger::INFO__, link.get()->strand.get()->logger().level);
  EXPECT_EQ(2, link.get()->get_app().get_thread_size());
  EXPECT_STREQ("mydslink", link.get()->dsid_prefix.c_str());
  EXPECT_EQ(132, link.get()->tcp_server_port);
  EXPECT_STREQ("192.168.1.12", link.get()->ws_host.c_str());
  EXPECT_EQ(142, link.get()->ws_port);
  end_link(std::move(link));
}
