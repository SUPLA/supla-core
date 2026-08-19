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

#include "device/channel_config_sync_coordinator.h"

#include <time.h>
#include <sys/time.h>
#include <vector>

#include "device/devicechannel.h"
#include "lck.h"

using std::vector;

supla_channel_config_sync_coordinator::supla_channel_config_sync_coordinator(
    unsigned _supla_int64_t batch_timeout_usec) {
  channels = nullptr;
  state = sync_state_not_started;
  batch_pos = 0;
  pending_result_count = 0;
  current_channel_number = -1;
  batch_started_at_usec = 0;
  this->batch_timeout_usec = batch_timeout_usec;
  lck = lck_init();
}

supla_channel_config_sync_coordinator::~supla_channel_config_sync_coordinator(
    void) {
  lck_free(lck);
}

unsigned _supla_int64_t supla_channel_config_sync_coordinator::now_usec(void) {
#ifdef CLOCK_MONOTONIC
  struct timespec monotonic_now = {};
  if (clock_gettime(CLOCK_MONOTONIC, &monotonic_now) == 0) {
    return monotonic_now.tv_sec * 1000000ULL +
           monotonic_now.tv_nsec / 1000ULL;
  }
#endif

  struct timeval now = {};
  gettimeofday(&now, nullptr);

  return now.tv_sec * 1000000ULL + now.tv_usec;
}

void supla_channel_config_sync_coordinator::reset_batch(void) {
  pending_result_count = 0;
  current_channel_number = -1;
  batch_started_at_usec = 0;
}

supla_channel_config_sync_coordinator::sync_step
supla_channel_config_sync_coordinator::finish(void) {
  sync_step step = {};

  if (state == sync_state_finished) {
    return step;
  }

  state = sync_state_finished;
  reset_batch();
  step.on_finished = on_finished;

  return step;
}

void supla_channel_config_sync_coordinator::start(
    vector<supla_device_channel *> *channels,
    std::function<void(void)> on_finished) {
  sync_step step = {};

  lck_lock(lck);

  if (state != sync_state_not_started) {
    lck_unlock(lck);
    return;
  }

  state = sync_state_sending;
  this->channels = channels;
  this->on_finished = on_finished;
  batch_pos = 0;
  step = get_next_step();
  lck_unlock(lck);

  execute_step(step);
}

supla_channel_config_sync_coordinator::sync_step
supla_channel_config_sync_coordinator::get_next_step(void) {
  sync_step step = {};
  reset_batch();

  if (!channels) {
    return finish();
  }

  while (batch_pos < channels->size()) {
    supla_device_channel *channel = (*channels)[batch_pos++];

    if (!channel) {
      continue;
    }

    if (!channel->prepare_config_for_device(&step.configs)) {
      continue;
    }

    step.channel = channel;
    step.wait_for_result = step.configs.size() > 0;
    state = sync_state_sending;

    if (step.wait_for_result) {
      pending_result_count = step.configs.size();
      current_channel_number = channel->get_channel_number();
      batch_started_at_usec = now_usec();
    }

    return step;
  }

  return finish();
}

void supla_channel_config_sync_coordinator::execute_step(
    sync_step step) {
  while (step.channel || step.on_finished) {
    if (step.channel) {
      step.channel->send_configs_to_device(&step.configs);
    }

    if (step.on_finished) {
      step.on_finished();
      return;
    }

    if (step.wait_for_result) {
      lck_lock(lck);

      if (pending_result_count > 0) {
        state = sync_state_waiting_for_result;
        lck_unlock(lck);
        return;
      }

      step = get_next_step();
      lck_unlock(lck);
      continue;
    }

    lck_lock(lck);
    step = get_next_step();
    lck_unlock(lck);
  }
}

void supla_channel_config_sync_coordinator::on_set_channel_config_result(
    TSDS_SetChannelConfigResult *result) {
  sync_step step = {};

  lck_lock(lck);

  if ((state != sync_state_sending &&
       state != sync_state_waiting_for_result) ||
      !result || pending_result_count == 0 ||
      result->ChannelNumber != current_channel_number) {
    lck_unlock(lck);
    return;
  }

  if (pending_result_count > 0) {
    pending_result_count--;
  }

  if (pending_result_count == 0 &&
      state == sync_state_waiting_for_result) {
    state = sync_state_sending;
    step = get_next_step();
  }

  lck_unlock(lck);

  execute_step(step);
}

void supla_channel_config_sync_coordinator::iterate(void) {
  sync_step step = {};

  lck_lock(lck);

  if (state != sync_state_waiting_for_result) {
    lck_unlock(lck);
    return;
  }

  if (now_usec() - batch_started_at_usec >= batch_timeout_usec) {
    state = sync_state_sending;
    step = get_next_step();
  }

  lck_unlock(lck);

  execute_step(step);
}

unsigned _supla_int64_t supla_channel_config_sync_coordinator::time_left_usec(
    void) {
  lck_lock(lck);

  if (state != sync_state_waiting_for_result) {
    lck_unlock(lck);
    return 0;
  }

  unsigned _supla_int64_t elapsed = now_usec() - batch_started_at_usec;

  if (elapsed >= batch_timeout_usec) {
    lck_unlock(lck);
    return 1;
  }

  unsigned _supla_int64_t result = batch_timeout_usec - elapsed;
  lck_unlock(lck);

  return result;
}
