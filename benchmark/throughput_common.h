#ifndef DSA_BENCHMARK_THROUGHPUT_H
#define DSA_BENCHMARK_THROUGHPUT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa/message.h"
#include "dsa/network.h"
#include "dsa/responder.h"
#include "dsa/stream.h"

#include "../test/sdk/async_test.h"
#include "../test/sdk/test_config.h"

#include "core/client.h"
#include "network/tcp/tcp_server.h"

#include <atomic>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/program_options.hpp>
#include <cassert>
#include <chrono>
#include <ctime>
#include <iostream>

using high_resolution_clock = std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;
using namespace boost::interprocess;
using namespace dsa;
namespace opts = boost::program_options;

class TestConfigExt : public TestConfig {
 public:
  TestConfigExt(std::shared_ptr<App> &app, std::string host_ip_address,
                int host_port, bool async = false)
      : TestConfig(app, async) {
    tcp_host = host_ip_address;
    tcp_server_port = host_port;
  }
};

class MockNode : public NodeModelBase {
 public:
  explicit MockNode(const LinkStrandRef &strand) : NodeModelBase(strand){};
};

class MessageQueue : public message_queue {
 private:
  int client_count;
  std::string mq_name;

 public:
  MessageQueue(create_only_t create_only, const char *name,
               size_type max_num_msg, size_type max_msg_size)
      : message_queue(create_only, name, max_num_msg, max_msg_size),
        client_count(max_num_msg),
        mq_name(name) {}

  MessageQueue(open_only_t open_only, const char *name)
      : message_queue(open_only, name),
        mq_name(name) {
    client_count = this->get_max_msg();
    assert(client_count > 0);
  }

  ~MessageQueue() = default;

  void send_all() {
    try {
      for (int i = 0; i < client_count; ++i) {
        int j = 1;
        this->send(&j, sizeof(j), 0);
      }
    } catch (interprocess_exception &ex) {
      message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void sync(int value = 1) {
    try {
      this->send(&value, sizeof(value), 0);
    } catch (interprocess_exception &ex) {
      message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void wait_all() {
    try {
      // Open a message queue.
      unsigned int priority;
      message_queue::size_type recvd_size;

      for (int i = 0; i < client_count; ++i) {
        int number;
        this->receive(&number, sizeof(number), recvd_size, priority);
      }
    } catch (interprocess_exception &ex) {
      message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void wait() {
    try {
      // Open a message queue.
      unsigned int priority;
      message_queue::size_type recvd_size;

      int number;
      this->receive(&number, sizeof(number), recvd_size, priority);
    } catch (interprocess_exception &ex) {
      message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

};

static std::string sc_mq_name("sc_throughput_mqueue");
static std::string cs_mq_name("cs_throughput_mqueue");

#endif  // DSA_BENCHMARK_THROUGHPUT_H
