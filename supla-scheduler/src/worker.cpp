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

#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "action_turn_onoff.h"
#include "ipcclient.h"
#include "log.h"
#include "queue.h"
#include "sthread.h"
#include "tools.h"

s_worker::s_worker(queue *q) : s_abstract_worker(q) {}

s_worker::~s_worker(void) {}

int s_worker::get_channel_func(void) { return get_params()->channel_func; }

int s_worker::get_id(void) { return get_params()->id; }

bool s_worker::it_applies_to_scene(void) { return get_params()->scene_id; }

bool s_worker::channel_group(void) {
  return get_params()->channel_group_id > 0;
}

int s_worker::get_retry_count(void) { return get_params()->retry_count; }

const char *s_worker::get_action_param(void) {
  return get_params()->action_param;
}

bool s_worker::retry_when_fail(void) { return get_params()->retry_when_fail; }

char s_worker::ipcc_get_opening_sensor_value() {
  char value = -1;

  supla_channel sensor_channel;
  memset(&sensor_channel, 0, sizeof(supla_channel));

  sensor_channel.id = get_params()->channel_param2;

  if (sensor_channel.id != 0 && get_db()->get_channel(&sensor_channel) &&
      sensor_channel.param1 == get_params()->channel_id &&
      (sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNO ||
       sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNC) &&
      get_ipcc()->is_connected(get_params()->user_id,
                               sensor_channel.iodevice_id) ==
          IPC_RESULT_CONNECTED) {
    if (!get_ipcc()->get_char_value(get_params()->user_id,
                                    sensor_channel.iodevice_id,
                                    sensor_channel.id, &value)) {
      value = -1;
    } else {
      value = !!value;
      if (get_params()->channel_param3 == 1) {
        value = !value;
      }
    }
  }

  return value;
}

bool s_worker::ipcc_set_char_value(char value) {
  return get_ipcc()->set_char_value(
      get_params()->user_id, get_params()->iodevice_id,
      get_params()->channel_id, get_params()->channel_group_id, value);
}

bool s_worker::ipcc_get_char_value(char *value) {
  return get_ipcc()->get_char_value(get_params()->user_id,
                                    get_params()->iodevice_id,
                                    get_params()->channel_id, value);
}

bool s_worker::ipcc_get_rgbw_value(int *color, char *color_brightness,
                                   char *brightness) {
  return get_ipcc()->get_rgbw_value(
      get_params()->user_id, get_params()->iodevice_id,
      get_params()->channel_id, color, color_brightness, brightness);
}

bool s_worker::ipcc_set_rgbw_value(int color, char color_brightness,
                                   char brightness) {
  return get_ipcc()->set_rgbw_value(
      get_params()->user_id, get_params()->iodevice_id,
      get_params()->channel_id, get_params()->channel_group_id, color,
      color_brightness, brightness);
}

bool s_worker::ipcc_get_valve_value(TValve_Value *value) {
  return get_ipcc()->get_valve_value(get_params()->user_id,
                                     get_params()->iodevice_id,
                                     get_params()->channel_id, value);
}

bool s_worker::ipcc_set_digiglass_value(int active_bits, int mask) {
  return get_ipcc()->set_digiglass_value(
      get_params()->user_id, get_params()->iodevice_id,
      get_params()->channel_id, active_bits, mask);
}

bool s_worker::ipcc_get_digiglass_value(int *mask) {
  return get_ipcc()->get_digiglass_value(get_params()->user_id,
                                         get_params()->iodevice_id,
                                         get_params()->channel_id, mask);
}

bool s_worker::ipcc_action_copy(int sourceDeviceId, int sourceChannelId) {
  return get_ipcc()->action_copy(
      get_params()->user_id, get_params()->iodevice_id,
      get_params()->channel_id, get_params()->channel_group_id, sourceDeviceId,
      sourceChannelId);
}

bool s_worker::ipcc_execute_scene(void) {
  return get_ipcc()->execute_scene(get_params()->user_id,
                                   get_params()->scene_id);
}

bool s_worker::ipcc_interrupt_scene(void) {
  return get_ipcc()->interrupt_scene(get_params()->user_id,
                                     get_params()->scene_id);
}

bool s_worker::ipcc_interrupt_and_execute_scene(void) {
  return get_ipcc()->interrupt_and_execute_scene(get_params()->user_id,
                                                 get_params()->scene_id);
}

char s_worker::ipcc_is_connected(void) {
  return get_ipcc()->is_connected(get_params()->user_id,
                                  get_params()->iodevice_id);
}
