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
#include "worker.h"

s_worker::s_worker(queue *q) {
  db = new database();
  ipcc = new ipc_client();
  this->q = q;
}

s_worker::~s_worker(void) {
  delete db;
  delete ipcc;
}

void s_worker::execute(void *sthread) {
  if (!db->connect()) return;

  s_exec = q->get_job();

  while (s_exec.id && !sthread_isterminated(sthread)) {
    if (db->set_fetched(s_exec.id)) q->mark_fetched();

    s_worker_action *action =
        AbstractActionFactory::createByActionType(s_exec.action, this);

    if (action) {
      action->execute();
      delete action;
    } else {
      db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_CANCELLED);
      supla_log(LOG_ERR, "Action %i is not supported!", s_exec.action);
    }

    if (s_exec.action_param != NULL) free(s_exec.action_param);

    s_exec = q->get_job();
  }

  q->raise_loop_event();
}

database *s_worker::get_db(void) { return this->db; }

int s_worker::get_channel_func(void) { return s_exec.channel_func; }

int s_worker::get_id(void) { return s_exec.id; }

int s_worker::get_retry_count(void) { return s_exec.retry_count; }

const char *s_worker::get_action_param(void) { return s_exec.action_param; }

bool s_worker::retry_when_fail(void) { return s_exec.retry_when_fail; }

char s_worker::ipcc_get_opening_sensor_value() {
  char value = -1;

  supla_channel sensor_channel;
  memset(&sensor_channel, 0, sizeof(supla_channel));

  sensor_channel.id = s_exec.channel_param2;

  if (sensor_channel.id != 0 && db->get_channel(&sensor_channel) &&
      sensor_channel.param1 == s_exec.channel_id &&
      (sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNO ||
       sensor_channel.type == SUPLA_CHANNELTYPE_SENSORNC) &&
      ipcc->is_connected(s_exec.user_id, sensor_channel.iodevice_id) ==
          IPC_RESULT_CONNECTED) {
    if (!ipcc->get_char_value(s_exec.user_id, sensor_channel.iodevice_id,
                              sensor_channel.id, &value)) {
      value = -1;
    } else {
      value = !!value;
      if (s_exec.channel_param3 == 1) {
        value = !value;
      }
    }
  }

  return value;
}

bool s_worker::ipcc_set_char_value(char value) {
  return ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id,
                              s_exec.channel_id, value);
}

bool s_worker::ipcc_get_char_value(char *value) {
  return ipcc->get_char_value(s_exec.user_id, s_exec.iodevice_id,
                              s_exec.channel_id, value);
}

bool s_worker::ipcc_get_rgbw_value(int *color, char *color_brightness,
                                   char *brightness) {
  return ipcc->get_rgbw_value(s_exec.user_id, s_exec.iodevice_id,
                              s_exec.channel_id, color, color_brightness,
                              brightness);
}

bool s_worker::ipcc_set_rgbw_value(int color, char color_brightness,
                                   char brightness) {
  return ipcc->set_rgbw_value(s_exec.user_id, s_exec.iodevice_id,
                              s_exec.channel_id, color, color_brightness,
                              brightness);
}

char s_worker::ipcc_is_connected(void) {
  return ipcc->is_connected(s_exec.user_id, s_exec.iodevice_id);
}
