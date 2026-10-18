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

#include "device/calcfg_queue.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <algorithm>
#include <string>
#include <vector>

#include "cJSON.h"
#include "device.h"
#include "device/device_dao.h"
#include "helper/json_helper.h"
#include "lck.h"
#include "tools.h"

bool supla_device_calcfg_queue::parse_item(
    cJSON *json_item, supla_device_calcfg_queue::item *result,
    unsigned _supla_int64_t id) {
  if (!json_item || !result) {
    return false;
  }

  TSD_DeviceCalCfgRequest request = {};
  int value = 0;
  bool authorized = false;

  if (!supla_json_helper::get_int(json_item, "sender_id", &value)) {
    return false;
  }
  request.SenderID = value;

  if (!supla_json_helper::get_int(json_item, "channel_number", &value)) {
    return false;
  }
  request.ChannelNumber = value;

  if (!supla_json_helper::get_int(json_item, "command_code", &value) ||
      !supla_device_calcfg_queue::is_queueable_command(value)) {
    return false;
  }
  request.Command = value;

  if (!supla_json_helper::get_bool(json_item, "super_user_authorized",
                                   &authorized)) {
    return false;
  }
  request.SuperUserAuthorized = authorized ? 1 : 0;

  if (!supla_json_helper::get_int(json_item, "data_type_code", &value)) {
    return false;
  }
  request.DataType = value;

  if (!supla_json_helper::get_int(json_item, "data_size", &value) ||
      value < 0 || value > SUPLA_CALCFG_DATA_MAXSIZE) {
    return false;
  }
  request.DataSize = (unsigned _supla_int_t)value;

  cJSON *data_json = cJSON_GetObjectItem(json_item, "data");
  if (request.DataSize > 0) {
    const char *hex = cJSON_GetStringValue(data_json);
    size_t hex_size = hex ? strnlen(hex, request.DataSize * 2 + 1) : 0;
    if (!hex || hex_size != request.DataSize * 2 ||
        st_hex2bin(request.Data, hex, hex_size) != request.DataSize) {
      return false;
    }
  }

  std::string state;
  if (!supla_json_helper::get_string(json_item, "enqueue_status", &state)) {
    return false;
  }

  supla_device_calcfg_queue::item_state item_state =
      supla_device_calcfg_queue::item_state_waiting_to_send;
  if (state == "WAITING_FOR_RESULT") {
    item_state = supla_device_calcfg_queue::item_state_waiting_for_result;
  } else if (state != "WAITING_TO_SEND") {
    return false;
  }

  unsigned _supla_int64_t queued_at = 0;
  unsigned _supla_int64_t sent_at = 0;
  time_t queued_timestamp = 0;
  time_t sent_timestamp = 0;
  if (!supla_json_helper::get_zulu_time_from_object(json_item, "queued_at",
                                                    &queued_timestamp) ||
      !supla_json_helper::get_zulu_time_from_object(json_item, "sent_at",
                                                    &sent_timestamp, true)) {
    return false;
  }
  queued_at = (unsigned _supla_int64_t)queued_timestamp;
  sent_at = (unsigned _supla_int64_t)sent_timestamp;

  result->id = id;
  result->request = request;
  result->state = item_state;
  result->queued_at = queued_at;
  result->sent_at = sent_at;
  return true;
}

unsigned _supla_int64_t supla_device_calcfg_queue::current_time(void) {
  return (unsigned _supla_int64_t)time(nullptr);
}

void supla_device_calcfg_queue::lock(void) const { lck_lock(lck); }

void supla_device_calcfg_queue::unlock(void) const { lck_unlock(lck); }

bool supla_device_calcfg_queue::persist(void) {
  return persist(get_snapshot());
}

bool supla_device_calcfg_queue::persist(
    const supla_device_calcfg_queue::queue_snapshot &snapshot) {
  int user_id = get_user_id();
  int device_id = get_device_id();

  if (!user_id || !device_id) {
    return false;
  }

  char *json = to_json(snapshot.items);
  if (!json) {
    return false;
  }

  lck_lock(persist_lck);

  bool result = true;
  if (snapshot.revision > persisted_revision) {
    result = save_snapshot(user_id, device_id, json, snapshot.valid_until);
    if (result) {
      persisted_revision = snapshot.revision;
    }
  }

  lck_unlock(persist_lck);
  free(json);

  return result;
}

int supla_device_calcfg_queue::get_user_id(void) const {
  return device ? device->get_user_id() : 0;
}

int supla_device_calcfg_queue::get_device_id(void) const {
  return device ? device->get_id() : 0;
}

int supla_device_calcfg_queue::get_device_flags(void) const {
  return device ? device->get_flags() : 0;
}

unsigned _supla_int64_t
supla_device_calcfg_queue::get_current_valid_until(void) const {
  if (!device || !device->get_connection()) {
    return 0;
  }

  unsigned char activity_timeout =
      device->get_connection()->get_activity_timeout();
  if (activity_timeout == 0) {
    return 0;
  }

  return current_time() + activity_timeout;
}

bool supla_device_calcfg_queue::save_snapshot(
    int user_id, int device_id, const char *queue_json,
    unsigned _supla_int64_t valid_until) {
  if (!queue_json || !user_id || !device_id) {
    return false;
  }

  supla_mariadb_access_provider dba;
  supla_device_dao dao(&dba);
  return dao.set_calcfg_queue(user_id, device_id, queue_json,
                              (time_t)valid_until);
}

bool supla_device_calcfg_queue::same_slot(const TSD_DeviceCalCfgRequest &a,
                                          const TSD_DeviceCalCfgRequest &b) {
  return a.ChannelNumber == b.ChannelNumber && a.Command == b.Command &&
         a.DataType == b.DataType && a.SenderID == b.SenderID;
}

bool supla_device_calcfg_queue::same_request(const TSD_DeviceCalCfgRequest &a,
                                             const TSD_DeviceCalCfgRequest &b) {
  if (a.SenderID != b.SenderID || a.ChannelNumber != b.ChannelNumber ||
      a.Command != b.Command || a.DataType != b.DataType ||
      a.DataSize != b.DataSize || a.DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return false;
  }

  return memcmp(a.Data, b.Data, a.DataSize) == 0;
}

bool supla_device_calcfg_queue::same_result_slot(
    const item &item, const TDS_DeviceCalCfgResult &result) {
  return item.state == item_state_waiting_for_result &&
         item.request.ChannelNumber == result.ChannelNumber &&
         item.request.Command == result.Command &&
         item.request.SenderID == result.ReceiverID;
}

bool supla_device_calcfg_queue::send_calcfg_request_now(
    TSD_DeviceCalCfgRequest *request, const send_callback &send_now) {
  if (!request || !send_now) {
    return false;
  }

  return send_now(request);
}

const char *supla_device_calcfg_queue::command_to_string(_supla_int_t command) {
  switch (command) {
    case SUPLA_CALCFG_CMD_ENTER_CFG_MODE:
      return "ENTER_CFG_MODE";
    case SUPLA_CALCFG_CMD_RESET_TO_FACTORY_SETTINGS:
      return "RESET_TO_FACTORY_SETTINGS";
    case SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE:
      return "CHECK_FIRMWARE_UPDATE";
    case SUPLA_CALCFG_CMD_START_FIRMWARE_UPDATE:
      return "START_FIRMWARE_UPDATE";
    case SUPLA_CALCFG_CMD_START_SECURITY_UPDATE:
      return "START_SECURITY_UPDATE";
    case SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD:
      return "SET_CFG_MODE_PASSWORD";
    case SUPLA_CALCFG_CMD_IDENTIFY_DEVICE:
      return "IDENTIFY_DEVICE";
    case SUPLA_CALCFG_CMD_RESTART_DEVICE:
      return "RESTART_DEVICE";
    case SUPLA_CALCFG_CMD_TAKE_OCR_PHOTO:
      return "TAKE_OCR_PHOTO";
    case SUPLA_CALCFG_CMD_MUTE_ALARM_SOUND:
      return "MUTE_ALARM_SOUND";
  }

  return "UNKNOWN";
}

const char *supla_device_calcfg_queue::data_type_to_string(
    _supla_int_t data_type) {
  switch (data_type) {
    case 0:
      return "NONE";
    case SUPLA_CALCFG_DATATYPE_RS_SETTINGS:
      return "RS_SETTINGS";
    case SUPLA_CALCFG_DATATYPE_FB_SETTINGS:
      return "FB_SETTINGS";
  }

  return "UNKNOWN";
}

const char *supla_device_calcfg_queue::item_state_to_string(item_state state) {
  switch (state) {
    case item_state_waiting_to_send:
      return "WAITING_TO_SEND";
    case item_state_sending:
      return "WAITING_TO_SEND";
    case item_state_waiting_for_result:
      return "WAITING_FOR_RESULT";
  }

  return "UNKNOWN";
}

std::vector<supla_device_calcfg_queue::item>::iterator
supla_device_calcfg_queue::find_same_slot(
    const TSD_DeviceCalCfgRequest &request) {
  return std::find_if(items.begin(), items.end(),
                      [&request](const item &it) -> bool {
                        return same_slot(it.request, request);
                      });
}

std::vector<supla_device_calcfg_queue::item>::iterator
supla_device_calcfg_queue::find_result(const TDS_DeviceCalCfgResult &result) {
  return std::find_if(items.begin(), items.end(),
                      [&result](const item &it) -> bool {
                        return same_result_slot(it, result);
                      });
}

supla_device_calcfg_queue::queue_snapshot
supla_device_calcfg_queue::get_snapshot(void) const {
  lock();
  queue_snapshot result = {items, valid_until, revision};
  unlock();
  return result;
}

supla_device_calcfg_queue::item supla_device_calcfg_queue::make_item(
    const TSD_DeviceCalCfgRequest &request, unsigned _supla_int64_t id) {
  item result = {};
  result.id = id;
  result.request = request;
  result.state = item_state_waiting_to_send;
  result.queued_at = current_time();
  return result;
}

supla_device_calcfg_queue::supla_device_calcfg_queue(supla_device *device) {
  lck = lck_init();
  persist_lck = lck_init();
  this->device = device;
  next_item_id = 0;
  valid_until = 0;
  revision = 0;
  persisted_revision = 0;
}

supla_device_calcfg_queue::~supla_device_calcfg_queue(void) {
  lck_free(persist_lck);
  lck_free(lck);
}

bool supla_device_calcfg_queue::is_queueable_command(_supla_int_t command) {
  switch (command) {
    case SUPLA_CALCFG_CMD_ENTER_CFG_MODE:
    case SUPLA_CALCFG_CMD_RESET_TO_FACTORY_SETTINGS:
    case SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE:
    case SUPLA_CALCFG_CMD_START_FIRMWARE_UPDATE:
    case SUPLA_CALCFG_CMD_START_SECURITY_UPDATE:
    case SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD:
    case SUPLA_CALCFG_CMD_IDENTIFY_DEVICE:
    case SUPLA_CALCFG_CMD_RESTART_DEVICE:
    case SUPLA_CALCFG_CMD_TAKE_OCR_PHOTO:
    case SUPLA_CALCFG_CMD_MUTE_ALARM_SOUND:
      return true;
  }

  return false;
}

bool supla_device_calcfg_queue::is_important_for_sleepers(
    const TSD_DeviceCalCfgRequest &request) {
  if (request.ChannelNumber != -1) {
    return false;
  }

  switch (request.Command) {
    case SUPLA_CALCFG_CMD_ENTER_CFG_MODE:
    case SUPLA_CALCFG_CMD_RESTART_DEVICE:
    case SUPLA_CALCFG_CMD_IDENTIFY_DEVICE:
      return true;
  }

  return false;
}

bool supla_device_calcfg_queue::requires_response(
    const TSD_DeviceCalCfgRequest &request) {
  switch (request.Command) {
    case SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE:
    case SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD:
      return true;
  }

  return false;
}

bool supla_device_calcfg_queue::should_queue_calcfg(
    TSD_DeviceCalCfgRequest *request) const {
  if (!request || !is_queueable_command(request->Command)) {
    return false;
  }

  int flags = get_device_flags();

  if (!(flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED)) {
    return false;
  }

  if (flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED) {
    return true;
  }

  return is_important_for_sleepers(*request);
}

bool supla_device_calcfg_queue::single_item_queue(void) const {
  int flags = get_device_flags();
  return (flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED) &&
         !(flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
}

bool supla_device_calcfg_queue::sync_done_supported(void) const {
  return get_device_flags() & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
}

bool supla_device_calcfg_queue::empty(void) const {
  lock();
  bool result = items.empty();
  unlock();

  return result;
}

size_t supla_device_calcfg_queue::size(void) const {
  lock();
  size_t result = items.size();
  unlock();

  return result;
}

supla_device_calcfg_queue::enqueue_result supla_device_calcfg_queue::enqueue(
    const TSD_DeviceCalCfgRequest &request, bool single_item_queue) {
  enqueue_result result = {enqueue_status_rejected, 0, 0};

  if (!is_queueable_command(request.Command) ||
      request.DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
    return result;
  }

  lock();
  item new_item = make_item(request, ++next_item_id);
  unsigned _supla_int64_t new_valid_until = get_current_valid_until();
  if (new_valid_until != 0) {
    valid_until = new_valid_until;
  }

  if (single_item_queue) {
    result.status =
        items.empty() ? enqueue_status_queued : enqueue_status_overwritten;
    result.queued_at = new_item.queued_at;
    items.clear();
    items.push_back(new_item);
    ++revision;
    unlock();
    persist();
    return result;
  }

  std::vector<item>::iterator same_slot_it = find_same_slot(request);
  if (same_slot_it != items.end()) {
    if (same_slot_it->state == item_state_waiting_for_result) {
      result.status = enqueue_status_waiting_for_result;
      result.queued_at = same_slot_it->queued_at;
      result.sent_at = same_slot_it->sent_at;
      unlock();
      return result;
    }

    *same_slot_it = new_item;
    result.status = enqueue_status_overwritten;
    result.queued_at = new_item.queued_at;
    ++revision;
    unlock();
    persist();
    return result;
  }

  items.push_back(new_item);
  result.status = enqueue_status_queued;
  result.queued_at = new_item.queued_at;
  ++revision;

  unlock();
  persist();

  return result;
}

bool supla_device_calcfg_queue::send_calcfg_request(
    TSD_DeviceCalCfgRequest *request, const send_callback &send_now,
    unsigned _supla_int64_t *queued_at, bool *waiting_for_result,
    enqueue_status *queue_status) {
  if (queued_at) {
    *queued_at = 0;
  }

  if (waiting_for_result) {
    *waiting_for_result = false;
  }

  if (queue_status) {
    *queue_status = enqueue_status_rejected;
  }

  if (!request) {
    return false;
  }

  if (should_queue_calcfg(request)) {
    supla_device_calcfg_queue::enqueue_result result =
        enqueue(*request, single_item_queue());
    if (queue_status) {
      *queue_status = result.status;
    }

    switch (result.status) {
      case enqueue_status_queued:
      case enqueue_status_overwritten:
        if (queued_at) {
          *queued_at = result.queued_at;
        }

        return true;

      case enqueue_status_waiting_for_result:
        if (queued_at) {
          *queued_at = result.queued_at;
        }

        if (waiting_for_result) {
          *waiting_for_result = true;
        }

        return true;

      default:
        break;
    }

    return false;
  }

  return send_calcfg_request_now(request, send_now);
}

bool supla_device_calcfg_queue::send_queued_calcfg_requests(
    const send_callback &send_now) {
  if (!sync_done_supported()) {
    return false;
  }

  bool result = false;
  bool changed = false;
  item item_to_send = {};

  while (take_next_item_to_send(&item_to_send)) {
    if (!send_calcfg_request_now(&item_to_send.request, send_now)) {
      on_item_send_failed(item_to_send);
      if (changed) {
        persist();
      }
      return result;
    }

    result = true;
    changed = on_item_sent(item_to_send, current_time()) || changed;
  }

  if (changed) {
    persist();
  }

  return result;
}

bool supla_device_calcfg_queue::on_calcfg_result(
    TDS_DeviceCalCfgResult *result) {
  if (result) {
    return on_result(*result);
  }

  return false;
}

bool supla_device_calcfg_queue::load(void) {
  int user_id = get_user_id();
  int device_id = get_device_id();
  if (!user_id || !device_id) {
    return false;
  }

  supla_mariadb_access_provider dba;
  supla_device_dao dao(&dba);
  std::string queue_json;
  time_t stored_valid_until = 0;

  if (!dao.get_calcfg_queue(user_id, device_id, &queue_json,
                            &stored_valid_until)) {
    return false;
  }

  if (queue_json.empty()) {
    return true;
  }

  if (stored_valid_until <= (time_t)current_time()) {
    lock();
    items.clear();
    valid_until = 0;
    ++revision;
    unlock();
    persist();
    return true;
  }

  cJSON *root = cJSON_Parse(queue_json.c_str());
  if (!root || !cJSON_IsArray(root)) {
    cJSON_Delete(root);
    return false;
  }

  std::vector<item> loaded_items;
  bool valid = true;
  unsigned _supla_int64_t item_id = 0;
  int item_count = cJSON_GetArraySize(root);
  for (int index = 0; index < item_count; ++index) {
    item loaded_item = {};
    if (!parse_item(cJSON_GetArrayItem(root, index), &loaded_item, ++item_id)) {
      valid = false;
      break;
    }
    loaded_items.push_back(loaded_item);
  }
  cJSON_Delete(root);

  if (!valid) {
    return false;
  }

  lock();
  items = loaded_items;
  next_item_id = item_id;
  valid_until = (unsigned _supla_int64_t)stored_valid_until;
  ++revision;
  persisted_revision = revision;
  unlock();

  return true;
}

bool supla_device_calcfg_queue::refresh_valid_until(void) {
  unsigned _supla_int64_t new_valid_until = get_current_valid_until();
  if (new_valid_until == 0) {
    return false;
  }

  lock();
  if (items.empty()) {
    unlock();
    return true;
  }

  valid_until = new_valid_until;
  ++revision;
  unlock();

  return persist();
}

int supla_device_calcfg_queue::take_latest_calcfg_command(void) {
  lock();
  if (items.empty()) {
    unlock();
    return 0;
  }

  int result = items.back().request.Command;
  items.clear();
  valid_until = 0;
  ++revision;
  unlock();
  persist();

  return result;
}

size_t supla_device_calcfg_queue::get_calcfg_queue_size(void) const {
  return size();
}

bool supla_device_calcfg_queue::take_next_item_to_send(item *result) {
  if (!result) {
    return false;
  }

  lock();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->state == item_state_waiting_to_send) {
      it->state = item_state_sending;
      *result = *it;
      unlock();
      return true;
    }
  }

  unlock();
  return false;
}

bool supla_device_calcfg_queue::on_item_sent(
    const supla_device_calcfg_queue::item &sent_item,
    unsigned _supla_int64_t sent_at) {
  lock();
  bool result = false;

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->state != item_state_sending || it->id != sent_item.id) {
      continue;
    }

    if (requires_response(sent_item.request)) {
      it->state = item_state_waiting_for_result;
      it->sent_at = sent_at;
    } else {
      items.erase(it);
      if (items.empty()) {
        valid_until = 0;
      }
    }

    ++revision;
    result = true;
    break;
  }

  unlock();
  return result;
}

bool supla_device_calcfg_queue::on_item_send_failed(const item &sent_item) {
  lock();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->state == item_state_sending && it->id == sent_item.id) {
      it->state = item_state_waiting_to_send;
      unlock();
      return true;
    }
  }

  unlock();
  return false;
}

bool supla_device_calcfg_queue::on_result(
    const TDS_DeviceCalCfgResult &result) {
  lock();
  std::vector<item>::iterator it = find_result(result);
  if (it != items.end()) {
    items.erase(it);
    if (items.empty()) {
      valid_until = 0;
    }
    ++revision;
    unlock();
    persist();
    return true;
  }
  unlock();

  return false;
}

int supla_device_calcfg_queue::take_latest_command(void) {
  lock();
  if (items.empty()) {
    unlock();
    return 0;
  }

  int result = items.back().request.Command;
  items.clear();
  valid_until = 0;
  ++revision;
  unlock();
  persist();
  return result;
}

std::vector<supla_device_calcfg_queue::item> supla_device_calcfg_queue::get_all(
    void) const {
  lock();
  std::vector<item> result = items;
  unlock();

  return result;
}

char *supla_device_calcfg_queue::to_json(void) const {
  return to_json(get_snapshot().items);
}

char *supla_device_calcfg_queue::to_json(
    const std::vector<supla_device_calcfg_queue::item> &snapshot) const {
  cJSON *root = cJSON_CreateArray();
  if (!root) {
    return nullptr;
  }

  for (auto it = snapshot.begin(); it != snapshot.end(); ++it) {
    cJSON *json_item = cJSON_CreateObject();
    if (!json_item) {
      cJSON_Delete(root);
      return nullptr;
    }

    cJSON_AddNumberToObject(json_item, "sender_id", it->request.SenderID);
    cJSON_AddNumberToObject(json_item, "channel_number",
                            it->request.ChannelNumber);
    cJSON_AddStringToObject(json_item, "command",
                            command_to_string(it->request.Command));
    cJSON_AddNumberToObject(json_item, "command_code", it->request.Command);
    cJSON_AddBoolToObject(json_item, "super_user_authorized",
                          it->request.SuperUserAuthorized != 0);
    cJSON_AddStringToObject(json_item, "data_type",
                            data_type_to_string(it->request.DataType));
    cJSON_AddNumberToObject(json_item, "data_type_code", it->request.DataType);
    cJSON_AddNumberToObject(json_item, "data_size", it->request.DataSize);
    if (it->request.DataSize > 0) {
      char data_hex[SUPLA_CALCFG_DATA_MAXSIZE * 2 + 1] = {};
      st_bin2hex(data_hex, it->request.Data, it->request.DataSize);
      cJSON_AddStringToObject(json_item, "data", data_hex);
    }
    cJSON_AddStringToObject(json_item, "enqueue_status",
                            item_state_to_string(it->state));
    supla_json_helper::add_zulu_time_to_object(json_item, "queued_at",
                                               (time_t)it->queued_at);
    supla_json_helper::add_zulu_time_to_object(json_item, "sent_at",
                                               (time_t)it->sent_at);
    cJSON_AddBoolToObject(json_item, "requires_response",
                          requires_response(it->request));

    cJSON_AddItemToArray(root, json_item);
  }

  char *result = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);

  return result;
}
