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
  uint16_t peer_count;
  std::string mq_name;

 public:
  MessageQueue(create_only_t create_only, std::string name,
               size_type max_num_msg, size_type max_msg_size,
               uint16_t peer_count = 1)
      : message_queue(create_only, name.c_str(), max_num_msg, max_msg_size),
        peer_count(peer_count),
        mq_name(name) {}

  MessageQueue(open_only_t open_only, std::string name, uint16_t peer_count = 1)
      : message_queue(open_only, name.c_str()),
        mq_name(name),
        peer_count(peer_count) {}

  ~MessageQueue() { message_queue::remove(mq_name.c_str()); }

  void send() {
    uint32_t x = 0;
    send(x);
  }

  void send(uint32_t &value) {
    try {
      message_queue::send(&value, sizeof(value), 0);
    } catch (interprocess_exception &ex) {
      // message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void recv() {
    uint32_t x;
    recv(x);
  }

  void recv(uint32_t &value) {
    try {
      unsigned int priority;
      message_queue::size_type recvd_size;
      this->receive(&value, sizeof(value), recvd_size, priority);
    } catch (interprocess_exception &ex) {
      // message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void bcast(uint32_t &value) {
    try {
      for (int i = 0; i < peer_count; ++i) {
        message_queue::send(&value, sizeof(value), 0);
      }
    } catch (interprocess_exception &ex) {
      // message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }

  void scatter() {
    try {
      for (uint32_t i = 0; i < peer_count; ++i) {
        message_queue::send(&i, sizeof(i), 0);
      }
    } catch (interprocess_exception &ex) {
      // message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }
};

class MessageQueues {
 private:
  uint16_t peer_count;
  std::unordered_map<std::string, std::unique_ptr<message_queue>> mq_map;

 public:
  MessageQueues(const std::string base_name, size_t max_num_msg,
                size_t max_msg_size, uint16_t peer_count_)
      : peer_count(peer_count_) {
    assert(peer_count > 0 && peer_count <= 256);

    for (int i = 0; i < peer_count; ++i) {
      std::string mq_name = base_name + std::to_string(i);
      try {
        message_queue::remove(mq_name.c_str());
        mq_map[mq_name] = std::make_unique<message_queue>(
            create_only, mq_name.c_str(), max_num_msg, max_msg_size);
      } catch (interprocess_exception &ex) {
        message_queue::remove(mq_name.c_str());
        std::cout << ex.what() << std::endl;
      }
    }
  }

  ~MessageQueues() {
    for (auto it = mq_map.begin(); it != mq_map.end(); ++it) {
      message_queue::remove(it->first.c_str());
    }
  }

  void gather() {
    uint64_t x;
    gather(x);
  }

  void gather(uint64_t &total) {
    try {
      uint32_t number;
      unsigned int priority;
      message_queue::size_type recvd_size;
      for (auto it = mq_map.begin(); it != mq_map.end(); ++it) {
        it->second->receive(&number, sizeof(number), recvd_size, priority);
        total += number;
      }
    } catch (interprocess_exception &ex) {
      // message_queue::remove(mq_name.c_str());
      std::cout << ex.what() << std::endl;
    }
  }
};

static std::string sc_mq_name("sc_throughput_mqueue");
static std::string cs_mq_name_base("cs_throughput_mqueue");

#endif  // DSA_BENCHMARK_THROUGHPUT_H
