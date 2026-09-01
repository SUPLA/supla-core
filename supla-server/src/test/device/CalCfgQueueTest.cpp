/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "device/calcfg_queue.h"
#include "gtest/gtest.h"
#include "doubles/device/DeviceStub.h"

namespace testing {

namespace {

TSD_DeviceCalCfgRequest request(_supla_int_t command) {
  TSD_DeviceCalCfgRequest result = {};
  result.ChannelNumber = -1;
  result.Command = command;
  result.SuperUserAuthorized = 1;
  return result;
}

TSD_DeviceCalCfgRequest channel_request(unsigned char channel_number,
                                        _supla_int_t command) {
  TSD_DeviceCalCfgRequest result = request(command);
  result.ChannelNumber = channel_number;
  return result;
}

TSD_DeviceCalCfgRequest password_request(const char *password) {
  TSD_DeviceCalCfgRequest result =
      request(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD);
  result.DataSize = strnlen(password, SUPLA_CALCFG_DATA_MAXSIZE);
  memcpy(result.Data, password, result.DataSize);
  return result;
}

TDS_DeviceCalCfgResult result_for(const supla_device_calcfg_queue::item &item) {
  TDS_DeviceCalCfgResult result = {};
  result.ChannelNumber = item.request.ChannelNumber;
  result.Command = item.request.Command;
  result.ReceiverID = item.request.SenderID;
  return result;
}

}  // namespace

TEST(CalCfgQueueTest, enqueueAddsAllCommandsWithoutPriorities) {
  supla_device_calcfg_queue queue;

  queue.enqueue(password_request("new"), false);
  queue.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), false);
  queue.enqueue(request(SUPLA_CALCFG_CMD_RESTART_DEVICE), false);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(3U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD,
            items.at(0).request.Command);
  EXPECT_EQ(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE, items.at(1).request.Command);
  EXPECT_EQ(SUPLA_CALCFG_CMD_RESTART_DEVICE, items.at(2).request.Command);
}

TEST(CalCfgQueueTest, singleItemQueueKeepsOnlyNewestWaitingCommand) {
  supla_device_calcfg_queue queue;

  queue.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), true);
  supla_device_calcfg_queue::enqueue_result result =
      queue.enqueue(request(SUPLA_CALCFG_CMD_RESTART_DEVICE), true);

  EXPECT_EQ(supla_device_calcfg_queue::enqueue_status_overwritten,
            result.status);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_RESTART_DEVICE, items.at(0).request.Command);
  EXPECT_EQ(SUPLA_CALCFG_CMD_RESTART_DEVICE, queue.take_latest_command());
  EXPECT_TRUE(queue.empty());
}

TEST(CalCfgQueueTest, waitingToSendSameCommandAndSenderIsOverwritten) {
  supla_device_calcfg_queue queue;
  TSD_DeviceCalCfgRequest old_request = password_request("old");
  old_request.SenderID = 100;
  old_request.SuperUserAuthorized = 0;

  TSD_DeviceCalCfgRequest new_request = password_request("new");
  new_request.SenderID = 100;
  new_request.SuperUserAuthorized = 1;

  queue.enqueue(old_request, false);
  supla_device_calcfg_queue::enqueue_result result =
      queue.enqueue(new_request, false);

  EXPECT_EQ(supla_device_calcfg_queue::enqueue_status_overwritten,
            result.status);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(100, items.at(0).request.SenderID);
  EXPECT_EQ(1, items.at(0).request.SuperUserAuthorized);
  EXPECT_EQ(0, memcmp(items.at(0).request.Data, "new", 3));
}

TEST(CalCfgQueueTest, waitingToSendSameCommandWithDifferentSenderIsAdded) {
  supla_device_calcfg_queue queue;
  TSD_DeviceCalCfgRequest first_request = password_request("one");
  first_request.SenderID = 100;

  TSD_DeviceCalCfgRequest second_request = password_request("two");
  second_request.SenderID = 200;

  queue.enqueue(first_request, false);
  supla_device_calcfg_queue::enqueue_result result =
      queue.enqueue(second_request, false);

  EXPECT_EQ(supla_device_calcfg_queue::enqueue_status_queued, result.status);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(2U, items.size());
  EXPECT_EQ(100, items.at(0).request.SenderID);
  EXPECT_EQ(200, items.at(1).request.SenderID);
}

TEST(CalCfgQueueTest, importantForSleepersMatchesLegacyDeviceCommands) {
  EXPECT_TRUE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_ENTER_CFG_MODE)));
  EXPECT_TRUE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_RESTART_DEVICE)));
  EXPECT_TRUE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE)));

  EXPECT_FALSE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE)));
  EXPECT_FALSE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_START_FIRMWARE_UPDATE)));
  EXPECT_FALSE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_RESET_TO_FACTORY_SETTINGS)));
  EXPECT_FALSE(supla_device_calcfg_queue::is_important_for_sleepers(
      request(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD)));
  EXPECT_FALSE(supla_device_calcfg_queue::is_important_for_sleepers(
      channel_request(1, SUPLA_CALCFG_CMD_IDENTIFY_DEVICE)));
}

TEST(CalCfgQueueTest, onlySelectedCommandsAreQueueable) {
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_ENTER_CFG_MODE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_RESET_TO_FACTORY_SETTINGS));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_START_FIRMWARE_UPDATE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_START_SECURITY_UPDATE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_IDENTIFY_DEVICE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_RESTART_DEVICE));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_TAKE_OCR_PHOTO));
  EXPECT_TRUE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_MUTE_ALARM_SOUND));

  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_IDENTIFY_SUBDEVICE));
  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING));
  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_RECALIBRATE));
  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_RESET_COUNTERS));
  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_RESTART_SUBDEVICE));
  EXPECT_FALSE(supla_device_calcfg_queue::is_queueable_command(
      SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE));
}

TEST(CalCfgQueueTest, responseRequirementDoesNotDependOnSenderId) {
  TSD_DeviceCalCfgRequest identify = request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE);
  identify.SenderID = 123;

  EXPECT_FALSE(supla_device_calcfg_queue::requires_response(identify));
  EXPECT_TRUE(supla_device_calcfg_queue::requires_response(
      request(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE)));
  EXPECT_TRUE(supla_device_calcfg_queue::requires_response(
      request(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD)));
}

TEST(CalCfgQueueTest, toJsonExportsInspectableQueueStateWithoutData) {
  supla_device_calcfg_queue queue;
  TSD_DeviceCalCfgRequest password = password_request("secret-password");
  password.SenderID = 321;
  password.ChannelNumber = 7;
  password.DataType = SUPLA_CALCFG_DATATYPE_RS_SETTINGS;
  password.SuperUserAuthorized = 0;

  queue.enqueue(password, false);

  char *json = queue.to_json();
  ASSERT_NE(nullptr, json);

  std::string snapshot(json);
  free(json);

  EXPECT_NE(std::string::npos, snapshot.find("\"sender_id\":321"));
  EXPECT_NE(std::string::npos, snapshot.find("\"channel_number\":7"));
  EXPECT_NE(std::string::npos,
            snapshot.find("\"command\":\"SET_CFG_MODE_PASSWORD\""));
  EXPECT_NE(std::string::npos, snapshot.find("\"command_code\":9050"));
  EXPECT_NE(std::string::npos,
            snapshot.find("\"super_user_authorized\":false"));
  EXPECT_NE(std::string::npos,
            snapshot.find("\"data_type\":\"RS_SETTINGS\""));
  EXPECT_NE(std::string::npos, snapshot.find("\"data_type_code\":1000"));
  EXPECT_NE(std::string::npos, snapshot.find("\"data_size\":15"));
  EXPECT_NE(std::string::npos,
            snapshot.find("\"enqueue_status\":\"WAITING_TO_SEND\""));
  EXPECT_NE(std::string::npos, snapshot.find("\"queued_at\":\""));
  EXPECT_NE(std::string::npos, snapshot.find("\"sent_at\":null"));
  EXPECT_NE(std::string::npos, snapshot.find("\"requires_response\":true"));
  EXPECT_EQ(std::string::npos, snapshot.find("secret-password"));
}

TEST(CalCfgQueueTest, legacySleepQueueOnlyQueuesCommandsImportantForSleepers) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest password = password_request("new");
  TSD_DeviceCalCfgRequest identify = request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE);
  bool send_called = false;
  unsigned _supla_int64_t queued_at = 1;
  bool waiting_for_result = true;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED);

  EXPECT_TRUE(queue.send_calcfg_request(
      &password,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_TRUE(send_called);
  EXPECT_EQ(0U, queued_at);
  EXPECT_FALSE(waiting_for_result);
  EXPECT_TRUE(queue.empty());

  send_called = false;
  EXPECT_TRUE(queue.send_calcfg_request(
      &identify,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_FALSE(send_called);
  EXPECT_NE(0U, queued_at);
  EXPECT_FALSE(waiting_for_result);
  EXPECT_EQ(1U, queue.size());
}

TEST(CalCfgQueueTest, syncDoneSleepQueueQueuesCommandsNotImportantForSleepers) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest password = password_request("new");
  bool send_called = false;
  unsigned _supla_int64_t queued_at = 0;
  bool waiting_for_result = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);

  EXPECT_TRUE(queue.send_calcfg_request(
      &password,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_FALSE(send_called);
  EXPECT_NE(0U, queued_at);
  EXPECT_FALSE(waiting_for_result);
  EXPECT_EQ(1U, queue.size());
}

TEST(CalCfgQueueTest, takeLatestCalCfgCommandUsesNewestCommand) {
  supla_device_calcfg_queue queue;

  queue.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), false);
  queue.enqueue(request(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE), false);

  EXPECT_EQ(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE,
            queue.take_latest_calcfg_command());
  EXPECT_TRUE(queue.empty());
}

TEST(CalCfgQueueTest, transferToSingleItemQueueKeepsLatestCommand) {
  supla_device_calcfg_queue previous;
  DeviceStub current_device(nullptr);
  supla_device_calcfg_queue current(&current_device);

  current_device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED);

  previous.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), false);
  previous.enqueue(password_request("new"), false);
  previous.enqueue(request(SUPLA_CALCFG_CMD_RESTART_DEVICE), false);
  previous.enqueue(request(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE), false);

  current.take_calcfg_queue_from(&previous);

  EXPECT_TRUE(previous.empty());

  std::vector<supla_device_calcfg_queue::item> items = current.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE,
            items.at(0).request.Command);
}

TEST(CalCfgQueueTest, commandWithoutResponseIsRemovedAfterSuccessfulSend) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  bool send_called = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), false);

  queue.send_queued_calcfg_requests(
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request && request->Command == SUPLA_CALCFG_CMD_IDENTIFY_DEVICE;
      });

  EXPECT_TRUE(send_called);
  EXPECT_TRUE(queue.empty());
}

TEST(CalCfgQueueTest, failedSendKeepsCommandWaitingToSend) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  bool send_called = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE), false);

  queue.send_queued_calcfg_requests(
      [&send_called](TSD_DeviceCalCfgRequest *) -> bool {
        send_called = true;
        return false;
      });

  EXPECT_TRUE(send_called);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_to_send,
            items.at(0).state);
}

TEST(CalCfgQueueTest, queuedSendRequestReportsQueuedCommand) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest identify = request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE);
  bool send_called = false;
  unsigned _supla_int64_t queued_at = 0;
  bool waiting_for_result = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);

  EXPECT_TRUE(queue.send_calcfg_request(
      &identify,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_FALSE(send_called);
  EXPECT_NE(0U, queued_at);
  EXPECT_FALSE(waiting_for_result);
}

TEST(CalCfgQueueTest, sentCommandReportsWaitingForResult) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest old_request = password_request("old");
  TSD_DeviceCalCfgRequest new_request = password_request("new");
  bool send_called = false;
  unsigned _supla_int64_t queued_at = 0;
  bool waiting_for_result = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(old_request, false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  EXPECT_TRUE(queue.send_calcfg_request(
      &new_request,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_FALSE(send_called);
  EXPECT_NE(0U, queued_at);
  EXPECT_TRUE(waiting_for_result);
}

TEST(CalCfgQueueTest, commandWithResponseWaitsUntilResult) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  bool send_called = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password_request("new"), false);
  queue.send_queued_calcfg_requests(
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      });

  EXPECT_TRUE(send_called);
  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_for_result,
            items.at(0).state);
  EXPECT_NE(0U, items.at(0).sent_at);

  queue.on_result(result_for(items.at(0)));
  EXPECT_TRUE(queue.empty());
}

TEST(CalCfgQueueTest, waitingForResultCommandIsNotOverwritten) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password_request("old"), false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  supla_device_calcfg_queue::enqueue_result result =
      queue.enqueue(password_request("new"), false);

  EXPECT_EQ(supla_device_calcfg_queue::enqueue_status_waiting_for_result,
            result.status);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(0, memcmp(items.at(0).request.Data, "old", 3));
}

TEST(CalCfgQueueTest, waitingForResultAppliesOnlyToTheSameCommandSlot) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest password = password_request("old");
  TSD_DeviceCalCfgRequest identify =
      request(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE);
  bool send_called = false;
  unsigned _supla_int64_t queued_at = 0;
  bool waiting_for_result = false;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password, false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  EXPECT_TRUE(queue.send_calcfg_request(
      &identify,
      [&send_called](TSD_DeviceCalCfgRequest *request) -> bool {
        send_called = true;
        return request != nullptr;
      },
      &queued_at, &waiting_for_result));

  EXPECT_FALSE(send_called);
  EXPECT_NE(0U, queued_at);
  EXPECT_FALSE(waiting_for_result);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(2U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD,
            items.at(0).request.Command);
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_for_result,
            items.at(0).state);
  EXPECT_EQ(SUPLA_CALCFG_CMD_IDENTIFY_DEVICE, items.at(1).request.Command);
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_to_send,
            items.at(1).state);
}

TEST(CalCfgQueueTest, singleItemQueueOverwritesCommandWaitingForResult) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password_request("old"), false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  supla_device_calcfg_queue::enqueue_result result =
      queue.enqueue(request(SUPLA_CALCFG_CMD_RESTART_DEVICE), true);

  EXPECT_EQ(supla_device_calcfg_queue::enqueue_status_overwritten,
            result.status);

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_RESTART_DEVICE, items.at(0).request.Command);
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_to_send,
            items.at(0).state);
}

TEST(CalCfgQueueTest, takeAllAndAppendPreservesItemState) {
  DeviceStub previous_device(nullptr);
  supla_device_calcfg_queue previous(&previous_device);
  supla_device_calcfg_queue current;

  previous_device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                            SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  previous.enqueue(password_request("new"), false);
  previous.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  current.append(previous.take_all());

  EXPECT_TRUE(previous.empty());

  std::vector<supla_device_calcfg_queue::item> items = current.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD,
            items.at(0).request.Command);
  EXPECT_EQ(supla_device_calcfg_queue::item_state_waiting_for_result,
            items.at(0).state);
  EXPECT_NE(0U, items.at(0).sent_at);
}

TEST(CalCfgQueueTest, calcfgResultMatchesByChannelAndCommand) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);
  TSD_DeviceCalCfgRequest password = password_request("new");
  password.SenderID = 123;

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password, false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(1U, items.size());

  TDS_DeviceCalCfgResult result = result_for(items.at(0));
  result.ReceiverID = 124;
  queue.on_result(result);
  EXPECT_FALSE(queue.empty());

  result.ReceiverID = 123;
  queue.on_result(result);
  EXPECT_TRUE(queue.empty());
}

TEST(CalCfgQueueTest, calcfgResultsCanArriveOutOfOrder) {
  DeviceStub device(nullptr);
  supla_device_calcfg_queue queue(&device);

  device.set_flags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED |
                   SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  queue.enqueue(password_request("new"), false);
  queue.enqueue(request(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE), false);
  queue.send_queued_calcfg_requests(
      [](TSD_DeviceCalCfgRequest *request) -> bool {
        return request != nullptr;
      });

  std::vector<supla_device_calcfg_queue::item> items = queue.get_all();
  ASSERT_EQ(2U, items.size());
  ASSERT_EQ(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD,
            items.at(0).request.Command);
  ASSERT_EQ(SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE,
            items.at(1).request.Command);

  queue.on_result(result_for(items.at(1)));

  items = queue.get_all();
  ASSERT_EQ(1U, items.size());
  EXPECT_EQ(SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD,
            items.at(0).request.Command);

  queue.on_result(result_for(items.at(0)));
  EXPECT_TRUE(queue.empty());
}

}  // namespace testing
