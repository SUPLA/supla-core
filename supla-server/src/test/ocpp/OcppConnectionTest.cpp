// SPDX-FileCopyrightText: AC SOFTWARE SP. Z O.O.
// SPDX-License-Identifier: GPL-2.0-or-later

#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "ocpp/ocpp_gateway.h"
#include "ocpp/ocpp_task_queue.h"
#include "ocpp/ocpp_worker_pool.h"
#include "sthread.h"

namespace testing {

TEST(OcppConnectionTest, SendsLengthPrefixedJson) {
  int sockets[2] = {-1, -1};
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));

  supla_ocpp_gateway::global_instance()->configure(false, 64 * 1024);
  {
    supla_ocpp_connection connection(sockets[0]);
    ASSERT_TRUE(connection.send_message(
        nlohmann::json{{"id", 17}, {"type", "on"}, {"device", 12}}));

    uint32_t network_size = 0;
    ASSERT_EQ(sizeof(network_size),
              static_cast<size_t>(
                  recv(sockets[1], &network_size, sizeof(network_size), 0)));
    uint32_t size = ntohl(network_size);
    ASSERT_GT(size, 0U);

    std::string payload(size, '\0');
    size_t offset = 0;
    while (offset < payload.size()) {
      ssize_t received =
          recv(sockets[1], &payload[offset], payload.size() - offset, 0);
      ASSERT_GT(received, 0);
      offset += received;
    }

    nlohmann::json message = nlohmann::json::parse(payload);
    EXPECT_EQ(17, message.at("id"));
    EXPECT_EQ("on", message.at("type"));
    EXPECT_EQ(12, message.at("device"));
  }

  close(sockets[1]);
}

TEST(OcppConnectionTest, RejectsFrameLargerThanConfiguredLimit) {
  int sockets[2] = {-1, -1};
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));

  supla_ocpp_gateway::global_instance()->configure(false, 1024);
  {
    supla_ocpp_connection connection(sockets[0]);
    EXPECT_FALSE(connection.send_message(
        nlohmann::json{{"payload", std::string(2048, 'x')}}));
  }

  close(sockets[1]);
}

TEST(OcppConnectionTest, MalformedHelloDoesNotEscapeThread) {
  for (const auto &hello :
       {nlohmann::json{{"type", 1}, {"version", 1}, {"validity", 90}},
        nlohmann::json{{"type", "hello"}, {"version", 1}, {"validity", -1}}}) {
    int sockets[2];
    ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
    auto connection = std::make_shared<supla_ocpp_connection>(sockets[0]);
    {
      supla_ocpp_connection sender(sockets[1]);
      ASSERT_TRUE(sender.send_message(hello));
    }
    void *thread = nullptr;
    sthread_simple_run(
        [](void *data, void *thread) {
          static_cast<supla_ocpp_connection *>(data)->execute(thread);
        },
        connection.get(), 0, &thread);
    sthread_wait(thread);
    sthread_free(thread);
  }
}

TEST(OcppConnectionTest, FailedWriteClosesConnectionAndRejectsLaterWrites) {
  int sockets[2];
  ASSERT_EQ(0, socketpair(AF_UNIX, SOCK_STREAM, 0, sockets));
  supla_ocpp_connection connection(sockets[0]);
  close(sockets[1]);
  EXPECT_FALSE(connection.send_message(nlohmann::json{{"id", 1}}));
  EXPECT_FALSE(connection.send_message(nlohmann::json{{"id", 2}}));
}

TEST(OcppTaskQueueTest, ReentrantCallbackRunsAfterCurrentOperation) {
  supla_ocpp_task_queue task_queue;
  std::vector<int> order;
  task_queue.post([&]() {
    order.push_back(1);
    task_queue.post([&]() { order.push_back(3); });
    order.push_back(2);
  });
  EXPECT_EQ((std::vector<int>{1, 2, 3}), order);
}

TEST(OcppTaskQueueTest, ConcurrentCallerQueuesWithoutWaitingForCallback) {
  supla_ocpp_task_queue task_queue;
  std::promise<void> started;
  std::promise<void> release;
  auto released = release.get_future();
  std::vector<int> order;
  std::thread drainer([&]() {
    task_queue.post([&]() {
      order.push_back(1);
      started.set_value();
      released.wait();
      order.push_back(2);
    });
  });
  started.get_future().wait();
  auto queued = std::async(std::launch::async, [&]() {
    task_queue.post([&]() { order.push_back(3); });
  });
  EXPECT_EQ(std::future_status::ready,
            queued.wait_for(std::chrono::seconds(1)));
  release.set_value();
  queued.wait();
  drainer.join();
  EXPECT_EQ((std::vector<int>{1, 2, 3}), order);
}

TEST(OcppTaskQueueTest, ExceptionDoesNotLeaveQueueLocked) {
  supla_ocpp_task_queue task_queue;
  int completed = 0;
  task_queue.post([&]() {
    task_queue.post([&]() { completed++; });
    throw std::runtime_error("test");
  });
  task_queue.post([&]() { completed++; });
  EXPECT_EQ(2, completed);
}

TEST(OcppWorkerPoolTest, PreservesDeviceOrderAndRunsShardsInParallel) {
  supla_ocpp_worker_pool workers;
  std::promise<void> first_started;
  std::promise<void> release_first;
  std::promise<void> same_device_finished;
  std::promise<void> other_device_finished;
  auto released = release_first.get_future();
  auto same_finished = same_device_finished.get_future();
  auto other_finished = other_device_finished.get_future();

  ASSERT_TRUE(workers.post(1, [&]() {
    first_started.set_value();
    released.wait();
  }));
  first_started.get_future().wait();
  ASSERT_TRUE(workers.post(5, [&]() { same_device_finished.set_value(); }));
  ASSERT_TRUE(workers.post(2, [&]() { other_device_finished.set_value(); }));

  EXPECT_EQ(std::future_status::ready,
            other_finished.wait_for(std::chrono::seconds(1)));
  EXPECT_EQ(std::future_status::timeout,
            same_finished.wait_for(std::chrono::milliseconds(20)));
  release_first.set_value();
  workers.wait_until_idle();
  EXPECT_EQ(std::future_status::ready,
            same_finished.wait_for(std::chrono::seconds(0)));
}

}  // namespace testing
