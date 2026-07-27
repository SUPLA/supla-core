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

#ifndef CHANNEL_CONFIG_SYNC_COORDINATOR_H_
#define CHANNEL_CONFIG_SYNC_COORDINATOR_H_

#include <stddef.h>

#include <functional>
#include <vector>

#include "proto.h"

class supla_device_channel;
class supla_channel_config_sync_coordinator {
 private:
  enum sync_state {
    sync_state_not_started,
    sync_state_sending,
    sync_state_waiting_for_result,
    sync_state_finished
  };

  struct sync_step {
    supla_device_channel *channel;
    std::vector<TSDS_SetChannelConfig> configs;
    bool wait_for_result;
    std::function<void(void)> on_finished;
  };

  std::vector<supla_device_channel *> *channels;
  sync_state state;
  size_t batch_pos;
  unsigned int pending_result_count;
  int current_channel_number;
  unsigned _supla_int64_t batch_started_at_usec;
  unsigned _supla_int64_t batch_timeout_usec;
  std::function<void(void)> on_finished;
  void *lck;

  unsigned _supla_int64_t now_usec(void);
  sync_step finish(void);
  void reset_batch(void);
  sync_step get_next_step(void);
  void execute_step(sync_step step);

 public:
  static const unsigned _supla_int64_t DEFAULT_BATCH_TIMEOUT_USEC = 2000000;

  explicit supla_channel_config_sync_coordinator(
      unsigned _supla_int64_t batch_timeout_usec =
          DEFAULT_BATCH_TIMEOUT_USEC);
  virtual ~supla_channel_config_sync_coordinator(void);

  void start(std::vector<supla_device_channel *> *channels,
             std::function<void(void)> on_finished);
  void on_set_channel_config_result(TSDS_SetChannelConfigResult *result);
  void iterate(void);
  unsigned _supla_int64_t time_left_usec(void);
};

#endif /* CHANNEL_CONFIG_SYNC_COORDINATOR_H_ */
