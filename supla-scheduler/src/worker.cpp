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
/*
void s_worker::set_result(bool success, int retry_limit, int retry_time,
                          bool no_sensor) {
  if (success) {
    // supla_log(LOG_DEBUG, "SUCCESS");
    db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_SUCCESS);

  } else if (s_exec.retry_count < retry_limit) {
    // supla_log(LOG_DEBUG, "RETRY %i", s_exec.retry_count);
    db->set_retry(s_exec.id, retry_time);

  } else if (no_sensor) {
    // supla_log(LOG_DEBUG, "NO SENSOR");
    db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_NO_SENSOR);

  } else {
    switch (ipcc->is_connected(s_exec.user_id, s_exec.iodevice_id)) {
      case IPC_RESULT_CONNECTED:
        db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_FAILURE);
        // supla_log(LOG_DEBUG, "RESULT_FAILURE");
        break;
      case IPC_RESULT_DISCONNECTED:
        db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_DEVICE_UNREACHABLE);
        // supla_log(LOG_DEBUG, "DEVICE_UNREACHABLE");
        break;
      case IPC_RESULT_SERVER_UNREACHABLE:
        db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_SERVER_UNREACHABLE);
        // supla_log(LOG_DEBUG, "SERVER_UNREACHABLE");
        break;
    }
  }
}

bool s_worker::check_function_allowed(int *func, int func_count) {
  int a;
  for (a = 0; a < func_count; a++)
    if (func[a] == s_exec.channel_func) return true;

  db->set_result(s_exec.id, ACTION_EXECUTION_RESULT_CANCELLED);
  return false;
}

void s_worker::action_turn_on_off(char on) {
  int func[] = {SUPLA_CHANNELFNC_LIGHTSWITCH, SUPLA_CHANNELFNC_POWERSWITCH};

  if (!check_function_allowed(func, sizeof(func) / sizeof(int))) return;

  bool success = ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id,
                                      s_exec.channel_id, on == 1 ? 1 : 0);
  set_result(success, ONOFF_RETRY_LIMIT, ONOFF_RETRY_TIME, false);
}

void s_worker::action_gate_open_close(char _close) {
  int func[] = {SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR,
                SUPLA_CHANNELFNC_CONTROLLINGTHEGATE};

  if (!check_function_allowed(func, sizeof(func) / sizeof(int))) return;

  // supla_log(LOG_DEBUG, "ACTION OPEN/CLOSE channel:%i try:%i",
  // s_exec.channel_id, s_exec.retry_count+1);

  bool success = false;
  char sensor_value = opening_sensor_value();

  if (sensor_value != -1) {
    if (sensor_value == _close) {
      success = true;
    } else if (s_exec.retry_count <
               GATEOPENCLOSE_RETRY_LIMIT) {  // last one only for sensor check
      ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id,
                           s_exec.channel_id, 1);
    }
  }

  set_result(success, GATEOPENCLOSE_RETRY_LIMIT, GATEOPENCLOSE_RETRY_TIME,
             sensor_value == -1);
}

void s_worker::action_open(void) {
  int func[] = {SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK,
                SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK};

  if (!check_function_allowed(func, sizeof(func) / sizeof(int))) return;

  bool success = ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id,
                                      s_exec.channel_id, 1);
  set_result(success, OPEN_RETRY_LIMIT, OPEN_RETRY_TIME, false);
}

void s_worker::action_shut_reveal(char shut) {
  bool success = false;
  char sensor_value = opening_sensor_value();
  char percent = 0;

  if (parse_percentage(&percent) == 1) {
    if (shut) {
      if (percent == 100) {
        percent = 0;
      } else if (percent == 0) {
        shut = 0;
        percent = 0;
      }

    } else {
      if (percent == 100) {
        percent = 0;
      } else if (percent == 0) {
        shut = 1;
        percent = 0;
      }
    }
  }

  char value = shut == 1 ? 1 : 2;

  if (percent > 0) {
    if (shut != 1) percent = 100 - percent;

    value = 10 + percent;

  } else if (value == 1) {
    percent = 100;
  }

  {
    char v = -2;
    ipcc->get_char_value(s_exec.user_id, s_exec.iodevice_id, s_exec.channel_id,
                         &v);

    success = (percent - v <= 2 && percent - v >= 0) ||
              (percent - v >= -2 && percent - v <= 0);  // tolerance 2%

    supla_log(LOG_DEBUG,
              "channel_id: %i, percent = %i, v = %i, sensor_value = %i, "
              "success = %i, diff = %i",
              s_exec.channel_id, percent, v, sensor_value, success,
              percent - v);

    // supla_log(LOG_DEBUG, "value = %i, percent = %i, v=%i, success = %i",
    // value, percent, v, success);

    if (success == true && sensor_value != -1 && sensor_value != 1 &&
        percent == 100)
      success = false;

    if (success == true && s_exec.retry_count == 0) {
      // supla_log(LOG_DEBUG, "s_exec.retry_count == 0", success);
      success = false;
    }

    // supla_log(LOG_DEBUG, "success = %i", success);
  }

  if (success == false)
    ipcc->set_char_value(s_exec.user_id, s_exec.iodevice_id, s_exec.channel_id,
                         value);

  set_result(success, RS_RETRY_LIMIT, RS_RETRY_TIME, false);
}



int s_worker::hue2rgb(double hue) {
  double r = 0, g = 0, b = 0;

  if (hue >= 360) hue = 0;

  hue /= 60.00;

  long i = (long)hue;
  double f, q, t;
  f = hue - i;

  q = 1.0 - f;
  t = 1.0 - (1.0 - f);

  switch (i) {
    case 0:
      r = 1.00;
      g = t;
      b = 0.00;
      break;

    case 1:
      r = q;
      g = 1.00;
      b = 0.00;
      break;

    case 2:
      r = 0.00;
      g = 1.00;
      b = t;
      break;

    case 3:
      r = 0.00;
      g = q;
      b = 1.00;
      break;

    case 4:
      r = t;
      g = 0.00;
      b = 1.00;
      break;

    default:
      r = 1.00;
      g = 0.00;
      b = q;
      break;
  }

  int rgb = 0;

  rgb |= (unsigned char)(r * 255.00);
  rgb <<= 8;

  rgb |= (unsigned char)(g * 255.00);
  rgb <<= 8;

  rgb |= (unsigned char)(b * 255.00);

  return rgb;
}

char s_worker::parse_percentage(char *percent) {
  jsmn_parser p;
  jsmntok_t t[10];
  int a;
  int result = 0;
  int value = 0;

  if (s_exec.action_param == NULL || percent == NULL) {
    return 0;
  }

  jsmn_init(&p);
  int r = jsmn_parse(&p, s_exec.action_param, strnlen(s_exec.action_param, 255),
                     t, sizeof(t) / sizeof(t[0]));

  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return 0;
  }

  for (a = 1; a < r - 1; a++) {
    if (jsoneq(s_exec.action_param, &t[a], "percentage") == 0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        *percent = value;
        return 1;
      }
    }
  }

  return result;
}

char s_worker::parse_rgbw_params(int *color, char *color_brightness,
                                 char *brightness) {
  jsmn_parser p;
  jsmntok_t t[10];
  int a;
  int result = 0;
  int value = 0;

  if (color) *color = 0;

  if (color_brightness) *color_brightness = 0;

  if (brightness) *brightness = 0;

  if (s_exec.action_param == NULL) {
    return 0;
  }

  jsmn_init(&p);
  int r = jsmn_parse(&p, s_exec.action_param, strnlen(s_exec.action_param, 255),
                     t, sizeof(t) / sizeof(t[0]));

  if (r < 1 || t[0].type != JSMN_OBJECT) {
    return 0;
  }

  for (a = 1; a < r - 1; a++) {
    if (jsoneq(s_exec.action_param, &t[a], "hue") == 0) {
      if (jsoneq(s_exec.action_param, &t[a + 1], "random") == 0) {
        if (color) {
          unsigned int seed = time(NULL);
          *color = hue2rgb(rand_r(&seed) % 360);
        }

        result++;
      }
      if (jsoneq(s_exec.action_param, &t[a + 1], "white") == 0) {
        if (color) {
          *color = 0xFFFFFF;
        }

        result++;

      } else if (json_get_int(&t[a + 1], &value)) {
        if (color) {
          *color = hue2rgb(value);
        }

        result++;
      }

    } else if (jsoneq(s_exec.action_param, &t[a], "color_brightness") == 0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        if (color_brightness) {
          *color_brightness = value;
        }

        result++;
      }
    } else if (jsoneq(s_exec.action_param, &t[a], "brightness") == 0) {
      if (json_get_int(&t[a + 1], &value) && value >= 0 && value <= 100) {
        if (brightness) {
          *brightness = value;
        }

        result++;
      }
    }
  }

  return result;
}

void s_worker::action_set_rgbw() {
  int color = 0;
  char color_brightness = 0;
  char brightness = 0;

  int func[] = {SUPLA_CHANNELFNC_DIMMER, SUPLA_CHANNELFNC_RGBLIGHTING,
                SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING};

  if (!check_function_allowed(func, sizeof(func) / sizeof(int))) return;

  parse_rgbw_params(&color, &color_brightness, &brightness);

  bool success = ipcc->set_rgbw_value(s_exec.user_id, s_exec.iodevice_id,
                                      s_exec.channel_id, color,
                                      color_brightness, brightness);
  set_result(success, RGBW_RETRY_LIMIT, RGBW_RETRY_TIME, false);
}
*/
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
    /*
    switch (s_exec.action) {
      case ACTION_CLOSE:
        action_gate_open_close(1);
        break;
      case ACTION_OPEN:

        if (s_exec.channel_func == SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR ||
            s_exec.channel_func == SUPLA_CHANNELFNC_CONTROLLINGTHEGATE) {
          action_gate_open_close(0);

        } else {
          action_open();
        }

        break;
      case ACTION_SHUT:
        action_shut_reveal(1);
        break;
      case ACTION_REVEAL:
      case ACTION_REVEAL_PARTIALLY:
        action_shut_reveal(0);
        break;
      case ACTION_TURN_ON:
        action_turn_on_off(1);
        s_worker_action_turn_onoff(this, true).execute();
        break;
      case ACTION_TURN_OFF:
        action_turn_on_off(0);
        break;
      case ACTION_SET_RGBW_PARAMETERS:
        action_set_rgbw();
        break;
        break;
    }
    */

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

char s_worker::ipcc_is_connected(void) {
  return ipcc->is_connected(s_exec.user_id, s_exec.iodevice_id);
}
