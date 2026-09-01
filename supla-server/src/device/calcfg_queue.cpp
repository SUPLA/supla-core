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

#include <string.h>
#include <time.h>

#include <algorithm>

#include "lck.h"

unsigned _supla_int64_t supla_device_calcfg_queue::current_time(void) {
  return (unsigned _supla_int64_t)time(nullptr);
}

void supla_device_calcfg_queue::lock(void) const { lck_lock(lck); }

void supla_device_calcfg_queue::unlock(void) const { lck_unlock(lck); }

bool supla_device_calcfg_queue::same_slot(const TSD_DeviceCalCfgRequest &a,
                                          const TSD_DeviceCalCfgRequest &b) {
  return a.ChannelNumber == b.ChannelNumber && a.Command == b.Command &&
         a.DataType == b.DataType && a.SenderID == b.SenderID;
}

bool supla_device_calcfg_queue::same_request(const TSD_DeviceCalCfgRequest &a,
                                             const TSD_DeviceCalCfgRequest &b) {
  if (a.SenderID != b.SenderID || a.ChannelNumber != b.ChannelNumber ||
      a.Command != b.Command || a.DataType != b.DataType ||
      a.DataSize != b.DataSize ||
      a.DataSize > SUPLA_CALCFG_DATA_MAXSIZE) {
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

supla_device_calcfg_queue::item supla_device_calcfg_queue::make_item(
    const TSD_DeviceCalCfgRequest &request) {
  item result = {};
  result.request = request;
  result.state = item_state_waiting_to_send;
  result.queued_at = current_time();
  return result;
}

supla_device_calcfg_queue::supla_device_calcfg_queue(void) {
  lck = lck_init();
  device_flags = 0;
}

supla_device_calcfg_queue::~supla_device_calcfg_queue(void) { lck_free(lck); }

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

void supla_device_calcfg_queue::set_device_flags(int device_flags) {
  lock();
  this->device_flags = device_flags;
  unlock();
}

bool supla_device_calcfg_queue::should_queue_calcfg(
    TSD_DeviceCalCfgRequest *request) const {
  if (!request || !is_queueable_command(request->Command)) {
    return false;
  }

  lock();
  int flags = device_flags;
  unlock();

  if (!(flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED)) {
    return false;
  }

  if (flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED) {
    return true;
  }

  return is_important_for_sleepers(*request);
}

bool supla_device_calcfg_queue::single_item_queue(void) const {
  lock();
  bool result = (device_flags & SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED) &&
                !(device_flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED);
  unlock();

  return result;
}

bool supla_device_calcfg_queue::sync_done_supported(void) const {
  lock();
  bool result = device_flags & SUPLA_DEVICE_FLAG_SYNC_DONE_SUPPORTED;
  unlock();

  return result;
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

  if (!is_queueable_command(request.Command)) {
    return result;
  }

  item new_item = make_item(request);

  lock();

  if (single_item_queue) {
    result.status =
        items.empty() ? enqueue_status_queued : enqueue_status_overwritten;
    result.queued_at = new_item.queued_at;
    items.clear();
    items.push_back(new_item);
    unlock();
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
    unlock();
    return result;
  }

  items.push_back(new_item);
  result.status = enqueue_status_queued;
  result.queued_at = new_item.queued_at;

  unlock();

  return result;
}

bool supla_device_calcfg_queue::send_calcfg_request(
    TSD_DeviceCalCfgRequest *request, const send_callback &send_now,
    unsigned _supla_int64_t *queued_at, bool *waiting_for_result) {
  if (queued_at) {
    *queued_at = 0;
  }

  if (waiting_for_result) {
    *waiting_for_result = false;
  }

  if (!request) {
    return false;
  }

  if (should_queue_calcfg(request)) {
    supla_device_calcfg_queue::enqueue_result result =
        enqueue(*request, single_item_queue());

    switch (result.status) {
      case enqueue_status_queued:
      case enqueue_status_overwritten:
        if (queued_at) {
          *queued_at = result.queued_at;
        }

        return true;

      case enqueue_status_waiting_for_result:
        if (queued_at) {
          *queued_at = result.sent_at ? result.sent_at : result.queued_at;
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

void supla_device_calcfg_queue::send_queued_calcfg_requests(
    const send_callback &send_now) {
  if (!sync_done_supported()) {
    return;
  }

  std::vector<supla_device_calcfg_queue::item> items = get_items_to_send();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (!send_calcfg_request_now(&it->request, send_now)) {
      return;
    }

    on_item_sent(*it, current_time());
  }
}

void supla_device_calcfg_queue::on_calcfg_result(
    TDS_DeviceCalCfgResult *result) {
  if (result) {
    on_result(*result);
  }
}

void supla_device_calcfg_queue::take_calcfg_queue_from(
    supla_device_calcfg_queue *queue) {
  if (!queue || queue == this) {
    return;
  }

  std::vector<supla_device_calcfg_queue::item> source_items = queue->take_all();
  if (source_items.empty()) {
    return;
  }

  bool use_single_item_queue = single_item_queue();

  lock();
  if (use_single_item_queue) {
    items.clear();
    items.push_back(source_items.back());
  } else {
    for (auto it = source_items.begin(); it != source_items.end(); ++it) {
      items.push_back(*it);
    }
  }
  unlock();
}

int supla_device_calcfg_queue::take_latest_calcfg_command(void) {
  lock();
  if (items.empty()) {
    unlock();
    return 0;
  }

  int result = items.back().request.Command;
  items.clear();
  unlock();

  return result;
}

size_t supla_device_calcfg_queue::get_calcfg_queue_size(void) const {
  return size();
}

std::vector<supla_device_calcfg_queue::item>
supla_device_calcfg_queue::get_items_to_send(void) const {
  std::vector<item> result;

  lock();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->state == item_state_waiting_to_send) {
      result.push_back(*it);
    }
  }

  unlock();

  return result;
}

void supla_device_calcfg_queue::on_item_sent(
    const supla_device_calcfg_queue::item &sent_item,
    unsigned _supla_int64_t sent_at) {
  lock();

  for (auto it = items.begin(); it != items.end(); ++it) {
    if (it->state != item_state_waiting_to_send ||
        it->queued_at != sent_item.queued_at ||
        !same_request(it->request, sent_item.request)) {
      continue;
    }

    if (requires_response(sent_item.request)) {
      it->state = item_state_waiting_for_result;
      it->sent_at = sent_at;
    } else {
      items.erase(it);
    }

    break;
  }

  unlock();
}

void supla_device_calcfg_queue::on_result(
    const TDS_DeviceCalCfgResult &result) {
  lock();
  std::vector<item>::iterator it = find_result(result);
  if (it != items.end()) {
    items.erase(it);
  }
  unlock();
}

int supla_device_calcfg_queue::take_latest_command(void) {
  lock();
  if (items.empty()) {
    unlock();
    return 0;
  }

  int result = items.back().request.Command;
  items.clear();
  unlock();
  return result;
}

std::vector<supla_device_calcfg_queue::item> supla_device_calcfg_queue::get_all(
    void) const {
  lock();
  std::vector<item> result = items;
  unlock();

  return result;
}

std::vector<supla_device_calcfg_queue::item>
supla_device_calcfg_queue::take_all(void) {
  lock();
  std::vector<item> result = items;
  items.clear();
  unlock();

  return result;
}

void supla_device_calcfg_queue::append(const std::vector<item> &items) {
  lock();
  for (auto it = items.begin(); it != items.end(); ++it) {
    this->items.push_back(*it);
  }
  unlock();
}
