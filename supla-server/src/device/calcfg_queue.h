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

#ifndef DEVICE_CALCFG_QUEUE_H_
#define DEVICE_CALCFG_QUEUE_H_

#include <stddef.h>

#include <functional>
#include <vector>

#include "proto.h"

class supla_device;
struct cJSON;

class supla_device_calcfg_queue {
 public:
  typedef std::function<bool(TSD_DeviceCalCfgRequest *request)> send_callback;

  enum item_state {
    item_state_waiting_to_send,
    item_state_sending,
    item_state_waiting_for_result
  };

  enum enqueue_status {
    enqueue_status_rejected,
    enqueue_status_queued,
    enqueue_status_overwritten,
    enqueue_status_waiting_for_result
  };

  struct enqueue_result {
    enqueue_status status;
    unsigned _supla_int64_t queued_at;
    unsigned _supla_int64_t sent_at;
  };

  struct item {
    unsigned _supla_int64_t id;
    TSD_DeviceCalCfgRequest request;
    item_state state;
    unsigned _supla_int64_t queued_at;
    unsigned _supla_int64_t sent_at;
  };

 private:
  struct queue_snapshot {
    std::vector<item> items;
    unsigned _supla_int64_t valid_until;
    unsigned _supla_int64_t revision;
  };

  std::vector<item> items;
  mutable void *lck;
  mutable void *persist_lck;
  supla_device *device;
  unsigned _supla_int64_t next_item_id;
  unsigned _supla_int64_t valid_until;
  unsigned _supla_int64_t revision;
  unsigned _supla_int64_t persisted_revision;

  void lock(void) const;
  void unlock(void) const;
  bool persist(void);
  bool persist(const queue_snapshot &snapshot);
  int get_user_id(void) const;
  int get_device_id(void) const;
  int get_device_flags(void) const;
  unsigned _supla_int64_t get_current_valid_until(void) const;
  bool save_snapshot(int user_id, int device_id, const char *queue_json,
                     unsigned _supla_int64_t valid_until);
  static unsigned _supla_int64_t current_time(void);
  static bool same_slot(const TSD_DeviceCalCfgRequest &a,
                        const TSD_DeviceCalCfgRequest &b);
  static bool same_request(const TSD_DeviceCalCfgRequest &a,
                           const TSD_DeviceCalCfgRequest &b);
  static bool same_result_slot(const item &item,
                               const TDS_DeviceCalCfgResult &result);
  static bool send_calcfg_request_now(TSD_DeviceCalCfgRequest *request,
                                      const send_callback &send_now);
  static const char *command_to_string(_supla_int_t command);
  static const char *data_type_to_string(_supla_int_t data_type);
  static const char *item_state_to_string(item_state state);
  static bool parse_item(struct cJSON *json_item, item *result,
                         unsigned _supla_int64_t id);
  bool take_next_item_to_send(item *result);
  bool on_item_sent(const item &sent_item, unsigned _supla_int64_t sent_at);
  bool on_item_send_failed(const item &sent_item);
  bool sync_done_supported(void) const;

  std::vector<item>::iterator find_same_slot(
      const TSD_DeviceCalCfgRequest &request);
  std::vector<item>::iterator find_result(const TDS_DeviceCalCfgResult &result);
  queue_snapshot get_snapshot(void) const;
  item make_item(const TSD_DeviceCalCfgRequest &request,
                 unsigned _supla_int64_t id);
  char *to_json(const std::vector<item> &items) const;
  supla_device_calcfg_queue(const supla_device_calcfg_queue &queue);
  supla_device_calcfg_queue &operator=(const supla_device_calcfg_queue &queue);

 public:
  explicit supla_device_calcfg_queue(supla_device *device = nullptr);
  virtual ~supla_device_calcfg_queue(void);

  static bool is_queueable_command(_supla_int_t command);
  static bool is_important_for_sleepers(const TSD_DeviceCalCfgRequest &request);
  static bool requires_response(const TSD_DeviceCalCfgRequest &request);
  bool should_queue_calcfg(TSD_DeviceCalCfgRequest *request) const;
  bool single_item_queue(void) const;

  bool empty(void) const;
  size_t size(void) const;
  enqueue_result enqueue(const TSD_DeviceCalCfgRequest &request,
                         bool single_item_queue);
  bool send_calcfg_request(TSD_DeviceCalCfgRequest *request,
                           const send_callback &send_now,
                           unsigned _supla_int64_t *queued_at = nullptr,
                           bool *waiting_for_result = nullptr,
                           enqueue_status *queue_status = nullptr);
  bool send_queued_calcfg_requests(const send_callback &send_now);
  bool on_calcfg_result(TDS_DeviceCalCfgResult *result);
  bool load(void);
  bool refresh_valid_until(void);
  int take_latest_calcfg_command(void);
  size_t get_calcfg_queue_size(void) const;
  bool on_result(const TDS_DeviceCalCfgResult &result);
  int take_latest_command(void);
  std::vector<item> get_all(void) const;
  char *to_json(void) const;
};

#endif /* DEVICE_CALCFG_QUEUE_H_ */
