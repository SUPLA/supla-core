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

#include <ipc/ipc_ctrl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "actions/action_executor.h"
#include "database.h"
#include "device/value_getter.h"
#include "http/httprequestqueue.h"
#include "ipcsocket.h"
#include "log.h"
#include "scene/scene_asynctask.h"
#include "serverstatus.h"
#include "sthread.h"
#include "tools.h"
#include "user.h"
#include "user/userchannelgroups.h"

// TODO(anyone): For setters, use the supla_action_executor class

const char hello[] = "SUPLA SERVER CTRL\n";
const char cmd_is_client_connected[] = "IS-CLIENT-CONNECTED:";
const char cmd_is_iodev_connected[] = "IS-IODEV-CONNECTED:";
const char cmd_is_channel_connected[] = "IS-CHANNEL-CONNECTED:";
const char cmd_is_channel_online[] = "IS-CHANNEL-ONLINE:";
const char cmd_user_reconnect[] = "USER-RECONNECT:";
const char cmd_client_reconnect[] = "CLIENT-RECONNECT:";
const char cmd_get_double_value[] = "GET-DOUBLE-VALUE:";
const char cmd_get_temperature_value[] = "GET-TEMPERATURE-VALUE:";
const char cmd_get_humidity_value[] = "GET-HUMIDITY-VALUE:";
const char cmd_get_char_value[] = "GET-CHAR-VALUE:";
const char cmd_get_rgbw_value[] = "GET-RGBW-VALUE:";
const char cmd_get_em_value[] = "GET-EM-VALUE:";
const char cmd_get_ic_value[] = "GET-IC-VALUE:";
const char cmd_get_valve_value[] = "GET-VALVE-VALUE:";
const char cmd_get_relay_value[] = "GET-RELAY-VALUE:";

const char cmd_set_char_value[] = "SET-CHAR-VALUE:";
const char cmd_set_rgbw_value[] = "SET-RGBW-VALUE:";
const char cmd_set_rand_rgbw_value[] = "SET-RAND-RGBW-VALUE:";

const char cmd_set_cg_char_value[] = "SET-CG-CHAR-VALUE:";
const char cmd_set_cg_rgbw_value[] = "SET-CG-RGBW-VALUE:";
const char cmd_set_cg_rand_rgbw_value[] = "SET-CG-RAND-RGBW-VALUE:";

const char cmd_set_digiglass_value[] = "SET-DIGIGLASS-VALUE:";
const char cmd_get_digiglass_value[] = "GET-DIGIGLASS-VALUE:";

const char cmd_action_open[] = "ACTION-OPEN:";
const char cmd_action_cg_open[] = "ACTION-CG-OPEN:";
const char cmd_action_close[] = "ACTION-CLOSE:";
const char cmd_action_cg_close[] = "ACTION-CG-CLOSE:";
const char cmd_action_toggle[] = "ACTION-TOGGLE:";
const char cmd_action_cg_toggle[] = "ACTION-CG-TOGGLE:";
const char cmd_action_stop[] = "ACTION-STOP:";
const char cmd_action_cg_stop[] = "ACTION-CG-STOP:";
const char cmd_action_copy[] = "ACTION-COPY:";
const char cmd_action_cg_copy[] = "ACTION-CG-COPY:";

const char cmd_action_up_or_stop[] = "ACTION-UP-OR-STOP:";
const char cmd_action_cg_up_or_stop[] = "ACTION-CG-UP-OR-STOP:";
const char cmd_action_down_or_stop[] = "ACTION-DOWN-OR-STOP:";
const char cmd_action_cg_down_or_stop[] = "ACTION-CG-DOWN-OR-STOP:";
const char cmd_action_step_by_step[] = "ACTION-SBS:";
const char cmd_action_cg_step_by_step[] = "ACTION-CG-SBS:";

const char cmd_reset_counters[] = "RESET-COUNTERS:";
const char cmd_recalibrate[] = "RECALIBRATE:";
const char cmd_get_status[] = "GET-STATUS";
const char cmd_enter_cfg_mode[] = "ACTION-ENTER-CONFIGURATION-MODE:";

const char cmd_execute_scene[] = "EXECUTE-SCENE:";
const char cmd_interrupt_scene[] = "INTERRUPT-SCENE:";

const char cmd_get_scene_summary[] = "GET-SCENE-SUMMARY:";
const char cmd_on_scene_added[] = "USER-ON-SCENE-ADDED:";
const char cmd_on_scene_deleted[] = "USER-ON-SCENE-DELETED:";
const char cmd_on_scene_changed[] = "USER-ON-SCENE-CHANGED:";

const char cmd_user_alexa_credentials_changed[] =
    "USER-ALEXA-CREDENTIALS-CHANGED:";

const char cmd_user_google_home_credentials_changed[] =
    "USER-GOOGLE-HOME-CREDENTIALS-CHANGED:";

const char cmd_user_state_webhook_changed[] = "USER-STATE-WEBHOOK-CHANGED:";

const char cmd_user_on_device_deleted[] = "USER-ON-DEVICE-DELETED:";

const char cmd_user_mqtt_settings_changed[] = "USER-MQTT-SETTINGS-CHANGED:";

const char cmd_user_before_device_delete[] = "USER-BEFORE-DEVICE-DELETE:";

const char cmd_user_on_device_settings_changed[] =
    "USER-ON-DEVICE-SETTINGS-CHANGED:";

const char cmd_user_before_channel_function_change[] =
    "USER-BEFORE-CHANNEL-FUNCTION-CHANGE:";

char ACT_VAR[] = ",ALEXA-CORRELATION-TOKEN=";
char GRI_VAR[] = ",GOOGLE-REQUEST-ID=";

supla_ipc_ctrl::supla_ipc_ctrl(int sfd) {
  this->sfd = sfd;

  this->eh = eh_init();
  eh_add_fd(eh, sfd);
}

bool supla_ipc_ctrl::match_command(const char *cmd, int len) {
  if (len > (int)strnlen(cmd, IPC_BUFFER_SIZE) &&
      memcmp(buffer, cmd, strnlen(cmd, IPC_BUFFER_SIZE)) == 0 &&
      buffer[len - 1] == '\n') {
    buffer[len - 1] = 0;
    return true;
  }

  return false;
}

void supla_ipc_ctrl::send_result(const char *result) {
  snprintf(buffer, sizeof(buffer), "%s\n", result);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void supla_ipc_ctrl::send_result(const char *result, int i) {
  snprintf(buffer, sizeof(buffer), "%s%i\n", result, i);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void supla_ipc_ctrl::send_result(const char *result, double i) {
  snprintf(buffer, sizeof(buffer), "%s%f\n", result, i);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void supla_ipc_ctrl::get_double(const char *cmd, char Type) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  double Value;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r = 0;

    switch (Type) {
      case 0:
        r = supla_user::get_channel_double_value(UserID, DeviceID, ChannelID,
                                                 &Value);
        break;
      case 1:
        r = supla_user::get_channel_temperature_value(UserID, DeviceID,
                                                      ChannelID, &Value);
        break;
      case 2:
        r = supla_user::get_channel_humidity_value(UserID, DeviceID, ChannelID,
                                                   &Value);
        break;
    }

    if (r) {
      send_result("VALUE:", Value);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_char(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  char Value;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r =
        supla_user::get_channel_char_value(UserID, DeviceID, ChannelID, &Value);

    if (r) {
      send_result("VALUE:", (int)Value);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_rgbw(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  int color;
  char color_brightness;
  char brightness;
  char on_off;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r = supla_user::get_channel_rgbw_value(UserID, DeviceID, ChannelID,
                                                &color, &color_brightness,
                                                &brightness, &on_off);

    if (r) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i,%i\n", color,
               color_brightness, brightness);
      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);

      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_impulsecounter_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    supla_channel_ic_measurement *icm = NULL;
    supla_user::access_device(
        UserID, DeviceID, ChannelID,
        [&icm, ChannelID](supla_device *device) -> void {
          icm = device->get_channels()->get_ic_measurement(ChannelID);
        });

    if (icm != NULL) {
      char *unit_b64 = st_openssl_base64_encode(
          (char *)icm->getCustomUnit(), strnlen(icm->getCustomUnit(), 9));

      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i,%i,%llu,%lld,%s,%s\n",
               icm->getTotalCost(), icm->getPricePerUnit(),
               icm->getImpulsesPerUnit(), icm->getCounter(),
               icm->getCalculatedValue(), icm->getCurrency(),
               unit_b64 ? unit_b64 : "");

      if (unit_b64) {
        free(unit_b64);
        unit_b64 = NULL;
      }

      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);

      delete icm;
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_electricitymeter_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    supla_channel_electricity_measurement *em = NULL;
    supla_user::access_device(
        UserID, DeviceID, ChannelID,
        [&em, ChannelID](supla_device *device) -> void {
          em = device->get_channels()->get_electricity_measurement(ChannelID);
        });

    if (em != NULL) {
      TElectricityMeter_ExtendedValue_V2 em_ev = {};
      char currency[4];
      em->getMeasurement(&em_ev);
      em->getCurrency(currency);

      unsigned int current1 = em_ev.m[0].current[0];
      unsigned int current2 = em_ev.m[0].current[1];
      unsigned int current3 = em_ev.m[0].current[2];

      if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
          !(em_ev.measured_values & EM_VAR_CURRENT)) {
        current1 *= 10;
        current2 *= 10;
        current3 *= 10;
      }

      _supla_int64_t power_active1 = em_ev.m[0].power_active[0];
      _supla_int64_t power_active2 = em_ev.m[0].power_active[1];
      _supla_int64_t power_active3 = em_ev.m[0].power_active[2];

      if (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW) {
        power_active1 *= 1000;
        power_active2 *= 1000;
        power_active3 *= 1000;
      }

      _supla_int64_t power_reactive1 = em_ev.m[0].power_reactive[0];
      _supla_int64_t power_reactive2 = em_ev.m[0].power_reactive[1];
      _supla_int64_t power_reactive3 = em_ev.m[0].power_reactive[2];

      if (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR) {
        power_reactive1 *= 1000;
        power_reactive2 *= 1000;
        power_reactive3 *= 1000;
      }

      _supla_int64_t power_apparent1 = em_ev.m[0].power_apparent[0];
      _supla_int64_t power_apparent2 = em_ev.m[0].power_apparent[1];
      _supla_int64_t power_apparent3 = em_ev.m[0].power_apparent[2];

      if (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA) {
        power_apparent1 *= 1000;
        power_apparent2 *= 1000;
        power_apparent3 *= 1000;
      }

      snprintf(buffer, sizeof(buffer),
               "VALUE:%i,%i,%i,%i,%i,%u,%u,%u,%lld,%lld,%lld,%lld,%lld,%lld,%"
               "lld,%lld,%lld,%i,%i,%i,%i,%i,%i,%llu,%llu,%llu,%llu,%llu,%llu,%"
               "llu,%llu,%llu,%llu,%llu,%llu,%i,%i,%s\n",
               em_ev.measured_values, em_ev.m[0].freq, em_ev.m[0].voltage[0],
               em_ev.m[0].voltage[1], em_ev.m[0].voltage[2], current1, current2,
               current3, power_active1, power_active2, power_active3,
               power_reactive1, power_reactive2, power_reactive3,
               power_apparent1, power_apparent2, power_apparent3,
               em_ev.m[0].power_factor[0], em_ev.m[0].power_factor[1],
               em_ev.m[0].power_factor[2], em_ev.m[0].phase_angle[0],
               em_ev.m[0].phase_angle[1], em_ev.m[0].phase_angle[2],
               em_ev.total_forward_active_energy[0],
               em_ev.total_forward_active_energy[1],
               em_ev.total_forward_active_energy[2],
               em_ev.total_reverse_active_energy[0],
               em_ev.total_reverse_active_energy[1],
               em_ev.total_reverse_active_energy[2],
               em_ev.total_forward_reactive_energy[0],
               em_ev.total_forward_reactive_energy[1],
               em_ev.total_forward_reactive_energy[2],
               em_ev.total_reverse_reactive_energy[0],
               em_ev.total_reverse_reactive_energy[1],
               em_ev.total_reverse_reactive_energy[2], em_ev.total_cost,
               em_ev.price_per_unit, currency);

      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);

      delete em;
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_valve_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  TValve_Value Value;
  memset(&Value, 0, sizeof(TValve_Value));

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r = supla_user::get_channel_valve_value(UserID, DeviceID, ChannelID,
                                                 &Value);

    if (r) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i", Value.closed,
               Value.flags);
      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_digiglass_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool result = false;
    unsigned short Mask = 0;

    supla_user::access_device(
        UserID, DeviceID, 0,
        [&result, ChannelID, &Mask](supla_device *device) -> void {
          result =
              device->get_channels()->get_dgf_transparency(ChannelID, &Mask);
        });

    if (result) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i", Mask);
      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::get_relay_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    TRelayChannel_Value value = {};
    bool result = false;

    supla_user::access_device(
        UserID, DeviceID, 0,
        [&result, ChannelID, &value](supla_device *device) -> void {
          result = device->get_channels()->get_relay_value(ChannelID, &value);
        });

    if (result) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i", value.hi, value.flags);
      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::reset_counters(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool result = false;
    supla_user::access_device(
        UserID, DeviceID, 0,
        [&result, ChannelID](supla_device *device) -> void {
          result = device->get_channels()->reset_counters(ChannelID);
        });

    if (result) {
      send_result("OK:", ChannelID);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::recalibrate(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool result = false;

    supla_user::access_device(
        UserID, DeviceID, 0,
        [&result, ChannelID](supla_device *device) -> void {
          result = device->get_channels()->recalibrate(
              ChannelID, supla_caller(ctIPC), true);
        });

    if (result) {
      send_result("OK:", ChannelID);
      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::enter_cfg_mode(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i", &UserID, &DeviceID);

  if (UserID && DeviceID) {
    bool result = false;

    supla_user::access_device(UserID, DeviceID, 0,
                              [&result](supla_device *device) -> void {
                                result = device->enter_cfg_mode();
                              });

    if (result) {
      send_result("OK:", DeviceID);
      return;
    }
  }

  send_result("UNKNOWN:", DeviceID);
}

void supla_ipc_ctrl::get_status(void) {
  if (!serverstatus::globalInstance()->getStatus(buffer, IPC_BUFFER_SIZE)) {
    supla_log(LOG_ERR, "%s", buffer);
  }
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void supla_ipc_ctrl::free_correlation_token() {
  if (AlexaCorrelationToken) {
    free(AlexaCorrelationToken);
    AlexaCorrelationToken = NULL;
  }
}

void supla_ipc_ctrl::free_google_requestid() {
  if (GoogleRequestId) {
    free(GoogleRequestId);
    GoogleRequestId = NULL;
  }
}

char *supla_ipc_ctrl::cut(const char *cmd, const char *var) {
  char *result = NULL;

  char *ct = strstr(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], var);
  unsigned int var_len = strnlen(var, 255);

  if (ct != NULL && strnlen(ct, IPC_BUFFER_SIZE) > var_len) {
    char *value = &ct[var_len];

    int len = strnlen(value, IPC_BUFFER_SIZE);

    if (len > 0) {
      result = st_openssl_base64_decode(value, len, NULL);
      if (strnlen(result, IPC_BUFFER_SIZE) <= 0) {
        delete result;
        result = NULL;
      }
    }

    ct[0] = 0;
  }

  return result;
}

void supla_ipc_ctrl::cut_correlation_token(const char *cmd) {
  free_correlation_token();
  AlexaCorrelationToken = cut(cmd, ACT_VAR);
}

void supla_ipc_ctrl::cut_google_requestid(const char *cmd) {
  free_google_requestid();
  GoogleRequestId = cut(cmd, GRI_VAR);
}

void supla_ipc_ctrl::execute_scene(const char *cmd) {
  int UserID = 0;
  int SceneID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i", &UserID, &SceneID);

  if (UserID && SceneID) {
    _sceneExecutionResult_e result = supla_scene_asynctask::execute(
        supla_scene_asynctask::get_queue(), supla_scene_asynctask::get_pool(),
        supla_caller(ctIPC), UserID, SceneID);

    if (result == serOK) {
      send_result("OK:", SceneID);
      return;
    } else if (result == serIsDuringExecution) {
      send_result("IS-DURING-EXECUTION:", SceneID);
      return;
    }
  }

  send_result("UNKNOWN:", SceneID);
}

void supla_ipc_ctrl::interrupt_scene(const char *cmd) {
  int UserID = 0;
  int SceneID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i", &UserID, &SceneID);

  if (UserID && SceneID) {
    supla_scene_asynctask::interrupt(supla_scene_asynctask::get_queue(), UserID,
                                     SceneID);
    send_result("OK:", SceneID);
  }

  send_result("UNKNOWN:", SceneID);
}

void supla_ipc_ctrl::set_char(const char *cmd, bool group) {
  int UserID = 0;
  int CGID = 0;
  int DeviceID = 0;
  int Value = 0;

  if (group) {
    sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &CGID,
           &Value);
  } else {
    cut_correlation_token(cmd);
    cut_google_requestid(cmd);

    sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i", &UserID,
           &DeviceID, &CGID, &Value);
  }

  supla_user *user = NULL;
  if (UserID && CGID && (user = supla_user::find(UserID, false)) != NULL) {
    if (Value < 0 || Value > IPC_BUFFER_SIZE) send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = user->get_channel_groups()->set_char_value(supla_caller(ctIPC),
                                                          CGID, Value);
    } else if (!group && DeviceID) {
      user->access_device(
          DeviceID, 0,
          [&result, user, DeviceID, CGID, Value,
           this](supla_device *device) -> void {
            // onChannelValueChangeEvent must be called before
            // set_device_channel_char_value for the potential report to contain
            // AlexaCorrelationToken / GoogleRequestId
            supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
                user, DeviceID, CGID,
                supla_caller(AlexaCorrelationToken
                                 ? ctAmazonAlexa
                                 : (GoogleRequestId ? ctGoogleHome : ctIPC)),
                AlexaCorrelationToken, GoogleRequestId);

            result = device->get_channels()->set_device_channel_char_value(
                supla_caller(ctIPC), CGID, 0, false, Value);
          });
    }

    free_correlation_token();
    free_google_requestid();

    if (result) {
      send_result("OK:", CGID);
    } else {
      send_result("FAIL:", CGID);
    }

    return;
  }

  send_result("UNKNOWN:", CGID);
}

void supla_ipc_ctrl::set_rgbw(const char *cmd, bool group, bool random) {
  int UserID = 0;
  int DeviceID = 0;
  int CGID = 0;
  int Color = 0;
  int ColorBrightness = 0;
  int Brightness = 0;
  int TurnOnOff = 0;

  if (random) {
    if (group) {
      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i", &UserID,
             &CGID, &ColorBrightness, &Brightness);

    } else {
      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i", &UserID,
             &DeviceID, &CGID, &ColorBrightness, &Brightness);
    }

    while (!Color) {
      struct timeval time;
      gettimeofday(&time, NULL);
      unsigned int seed = time.tv_sec + time.tv_usec;

      Color = st_hue2rgb(rand_r(&seed) % 360);
    }
  } else {
    if (group) {
      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i,%i",
             &UserID, &CGID, &Color, &ColorBrightness, &Brightness, &TurnOnOff);

    } else {
      cut_correlation_token(cmd);
      cut_google_requestid(cmd);

      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i,%i,%i",
             &UserID, &DeviceID, &CGID, &Color, &ColorBrightness, &Brightness,
             &TurnOnOff);
    }
  }

  supla_user *user = NULL;
  if (UserID && CGID && (user = supla_user::find(UserID, false)) != NULL) {
    if (ColorBrightness < 0 || ColorBrightness > 100 || Brightness < 0 ||
        Brightness > 100)
      send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = user->get_channel_groups()->set_rgbw_value(
          supla_caller(ctIPC), CGID, Color, ColorBrightness, Brightness,
          TurnOnOff);
    } else if (!group && DeviceID) {
      user->access_device(
          DeviceID, 0,
          [&result, this, user, DeviceID, CGID, Color, ColorBrightness,
           Brightness, TurnOnOff](supla_device *device) -> void {
            // onChannelValueChangeEvent must be called before
            // set_device_channel_char_value for the potential report to contain
            // AlexaCorrelationToken / GoogleRequestId
            supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
                user, DeviceID, CGID,
                supla_caller(AlexaCorrelationToken
                                 ? ctAmazonAlexa
                                 : (GoogleRequestId ? ctGoogleHome : ctIPC)),
                AlexaCorrelationToken, GoogleRequestId);

            result = device->get_channels()->set_device_channel_rgbw_value(
                supla_caller(ctIPC), CGID, 0, false, Color, ColorBrightness,
                Brightness, TurnOnOff);
          });
    }

    free_correlation_token();
    free_google_requestid();

    if (result) {
      send_result("OK:", CGID);
    } else {
      send_result("FAIL:", CGID);
    }

    return;
  }

  send_result("UNKNOWN:", CGID);
}

void supla_ipc_ctrl::set_digiglass_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  int ActiveBits = 0;
  int Mask = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i", &UserID,
         &DeviceID, &ChannelID, &ActiveBits, &Mask);

  if (UserID && DeviceID && ChannelID) {
    bool result = false;

    supla_user::access_device(
        UserID, DeviceID, 0,
        [&result, ChannelID, ActiveBits, Mask](supla_device *device) -> void {
          result = device->get_channels()->set_dgf_transparency(
              supla_caller(ctIPC), ChannelID, ActiveBits & 0xFFFF,
              Mask & 0xFFFF);
        });

    if (result) {
      send_result("OK:", ChannelID);
    } else {
      send_result("FAIL:", ChannelID);
    }
    return;
  }

  send_result("UNKNOWN:", ChannelID);
}

void supla_ipc_ctrl::action_open_close(const char *cmd, bool open) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  cut_correlation_token(cmd);
  cut_google_requestid(cmd);

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  supla_user *user = NULL;
  if (UserID && DeviceID && ChannelID &&
      (user = supla_user::find(UserID, false)) != NULL) {
    bool result = false;

    user->access_device(
        DeviceID, 0,
        [&result, this, user, DeviceID, ChannelID,
         open](supla_device *device) -> void {
          // onChannelValueChangeEvent must be called before
          // set_device_channel_char_value for the potential report to contain
          // AlexaCorrelationToken / GoogleRequestId
          supla_http_request_queue::getInstance()->onChannelValueChangeEvent(
              user, DeviceID, ChannelID,
              supla_caller(AlexaCorrelationToken
                               ? ctAmazonAlexa
                               : (GoogleRequestId ? ctGoogleHome : ctIPC)),
              AlexaCorrelationToken, GoogleRequestId);

          if (open) {
            result = device->get_channels()->action_open(supla_caller(ctIPC),
                                                         ChannelID, 0, 0);
          } else {
            result = device->get_channels()->action_close(supla_caller(ctIPC),
                                                          ChannelID);
          }
        });

    if (result) {
      send_result("OK:", ChannelID);
    } else {
      send_result("FAIL:", ChannelID);
    }

  } else {
    send_result("USER_UNKNOWN");
  }

  free_correlation_token();
  free_google_requestid();
}

void supla_ipc_ctrl::channel_groups_action(
    const char *cmd, std::function<bool(supla_user_channelgroups *, int)> f) {
  int UserID = 0;
  int GroupID = 0;
  bool result = false;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i", &UserID, &GroupID);

  supla_user *user = NULL;
  if (UserID && GroupID && (user = supla_user::find(UserID, false)) != NULL) {
    result = f(user->get_channel_groups(), GroupID);
  }

  if (result) {
    send_result("OK:", GroupID);
  } else {
    send_result("FAIL:", GroupID);
  }
}

void supla_ipc_ctrl::channel_action(
    const char *cmd, std::function<bool(supla_device_channels *, int)> f) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  bool result = false;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  supla_user::access_device(
      UserID, DeviceID, 0,
      [&result, ChannelID, f](supla_device *device) -> void {
        result = f(device->get_channels(), ChannelID);
      });

  if (result) {
    send_result("OK:", ChannelID);
  } else {
    send_result("FAIL:", ChannelID);
  }
}

void supla_ipc_ctrl::action_cg_open_close(const char *cmd, bool open) {
  channel_groups_action(
      cmd,
      [open](supla_user_channelgroups *channel_groups, int GroupID) -> bool {
        if (open) {
          return channel_groups->action_open(supla_caller(ctIPC), GroupID);
        } else {
          return channel_groups->action_close(supla_caller(ctIPC), GroupID);
        }
      });
}

void supla_ipc_ctrl::action_toggle(const char *cmd) {
  channel_action(
      cmd, [](supla_device_channels *channels, int channel_id) -> bool {
        return channels->action_toggle(supla_caller(ctIPC), channel_id, 0, 0);
      });
}

void supla_ipc_ctrl::action_cg_toggle(const char *cmd) {
  channel_groups_action(
      cmd, [](supla_user_channelgroups *channel_groups, int group_id) -> bool {
        return channel_groups->action_toggle(supla_caller(ctIPC), group_id);
      });
}

void supla_ipc_ctrl::action_stop(const char *cmd) {
  channel_action(
      cmd, [](supla_device_channels *channels, int channel_id) -> bool {
        return channels->action_stop(supla_caller(ctIPC), channel_id, 0, 0);
      });
}

void supla_ipc_ctrl::action_cg_stop(const char *cmd) {
  channel_groups_action(
      cmd, [](supla_user_channelgroups *channel_groups, int group_id) -> bool {
        return channel_groups->action_stop(supla_caller(ctIPC), group_id);
      });
}

void supla_ipc_ctrl::action_up_or_stop(const char *cmd) {
  channel_action(cmd,
                 [](supla_device_channels *channels, int channel_id) -> bool {
                   return channels->action_up_or_stop(supla_caller(ctIPC),
                                                      channel_id, 0, 0);
                 });
}

void supla_ipc_ctrl::action_cg_up_or_stop(const char *cmd) {
  channel_groups_action(
      cmd, [](supla_user_channelgroups *channel_groups, int group_id) -> bool {
        return channel_groups->action_up_or_stop(supla_caller(ctIPC), group_id);
      });
}

void supla_ipc_ctrl::action_down_or_stop(const char *cmd) {
  channel_action(cmd,
                 [](supla_device_channels *channels, int channel_id) -> bool {
                   return channels->action_down_or_stop(supla_caller(ctIPC),
                                                        channel_id, 0, 0);
                 });
}

void supla_ipc_ctrl::action_cg_down_or_stop(const char *cmd) {
  channel_groups_action(
      cmd, [](supla_user_channelgroups *channel_groups, int group_id) -> bool {
        return channel_groups->action_down_or_stop(supla_caller(ctIPC),
                                                   group_id);
      });
}

void supla_ipc_ctrl::action_step_by_step(const char *cmd) {
  channel_action(cmd,
                 [](supla_device_channels *channels, int channel_id) -> bool {
                   return channels->action_step_by_step(supla_caller(ctIPC),
                                                        channel_id, 0, 0);
                 });
}

void supla_ipc_ctrl::action_cg_step_by_step(const char *cmd) {
  channel_groups_action(
      cmd, [](supla_user_channelgroups *channel_groups, int group_id) -> bool {
        return channel_groups->action_step_by_step(supla_caller(ctIPC),
                                                   group_id);
      });
}

void supla_ipc_ctrl::action_copy(const char *cmd, bool group) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  int GroupID = 0;
  int SourceDeviceID = 0;
  int SourceChannelID = 0;
  bool result = false;

  if (group) {
    sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i", &UserID,
           &GroupID, &SourceDeviceID, &SourceChannelID);
  } else {
    sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i", &UserID,
           &DeviceID, &ChannelID, &SourceDeviceID, &SourceChannelID);
  }

  if (UserID && (ChannelID || GroupID)) {
    supla_action_executor *action_executor = new supla_action_executor();
    action_executor->set_caller(supla_caller(ctIPC));
    if (action_executor) {
      if (group) {
        action_executor->set_group_id(UserID, GroupID);
      } else {
        action_executor->set_channel_id(UserID, DeviceID, ChannelID);
      }

      supla_value_getter *value_getter = new supla_value_getter();
      if (value_getter) {
        action_executor->copy(value_getter, SourceDeviceID, SourceChannelID);
        result = true;
        delete value_getter;
        value_getter = NULL;
      }

      delete action_executor;
      action_executor = NULL;
    }
  }

  if (result) {
    send_result("OK:", ChannelID);
  } else {
    send_result("FAIL:", ChannelID);
  }
}

void supla_ipc_ctrl::action_copy(const char *cmd) { action_copy(cmd, false); }

void supla_ipc_ctrl::action_cg_copy(const char *cmd) { action_copy(cmd, true); }

void supla_ipc_ctrl::alexa_credentials_changed(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_alexa_credentials_changed, IPC_BUFFER_SIZE)],
         "%i", &UserID);

  if (UserID) {
    supla_user::on_amazon_alexa_credentials_changed(UserID);
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::google_home_credentials_changed(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_google_home_credentials_changed,
                         IPC_BUFFER_SIZE)],
         "%i", &UserID);
  if (UserID) {
    supla_user::on_google_home_credentials_changed(UserID);
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::state_webhook_changed(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_state_webhook_changed, IPC_BUFFER_SIZE)],
         "%i", &UserID);
  if (UserID) {
    supla_user::on_state_webhook_changed(UserID);
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::mqtt_settings_changed(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_mqtt_settings_changed, IPC_BUFFER_SIZE)],
         "%i", &UserID);
  if (UserID) {
    supla_user::on_mqtt_settings_changed(UserID);
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::before_channel_function_change(const char *cmd) {
  int UserID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd_user_before_channel_function_change,
                         IPC_BUFFER_SIZE)],
         "%i,%i", &UserID, &ChannelID);
  if (UserID && ChannelID) {
    supla_user::before_channel_function_change(UserID, ChannelID,
                                               supla_caller(ctIPC));
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::before_device_delete(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;

  sscanf(&buffer[strnlen(cmd_user_before_device_delete, IPC_BUFFER_SIZE)],
         "%i,%i", &UserID, &DeviceID);
  if (UserID && DeviceID) {
    supla_user::before_device_delete(UserID, DeviceID, supla_caller(ctIPC));
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::on_device_deleted(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;

  sscanf(&buffer[strnlen(cmd_user_on_device_deleted, IPC_BUFFER_SIZE)], "%i,%i",
         &UserID, &DeviceID);
  if (UserID && DeviceID) {
    supla_user::on_device_deleted(UserID, DeviceID, supla_caller(ctIPC));
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::on_device_settings_changed(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;

  sscanf(&buffer[strnlen(cmd_user_on_device_settings_changed, IPC_BUFFER_SIZE)],
         "%i,%i", &UserID, &DeviceID);
  if (UserID && DeviceID) {
    supla_user::on_device_settings_changed(UserID, DeviceID,
                                           supla_caller(ctIPC));
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void supla_ipc_ctrl::execute(void *sthread) {
  if (sfd == -1) return;

  int len;

  struct timeval last_action;
  gettimeofday(&last_action, NULL);

  send(sfd, hello, sizeof(hello), 0);

  while (sthread_isterminated(sthread) == 0) {
    eh_wait(eh, 1000000);
    memset(buffer, 0, IPC_BUFFER_SIZE);

    if ((len = recv(sfd, buffer, sizeof(buffer), 0)) != 0) {
      if (len > 0) {
        buffer[IPC_BUFFER_SIZE - 1] = 0;

        if (match_command(cmd_is_client_connected, len)) {
          int UserID = 0;
          int ClientID = 0;
          sscanf(&buffer[strnlen(cmd_is_client_connected, IPC_BUFFER_SIZE)],
                 "%i,%i", &UserID, &ClientID);

          if (UserID && ClientID &&
              supla_user::is_client_online(UserID, ClientID)) {
            send_result("CONNECTED:", ClientID);
          } else {
            send_result("DISCONNECTED:", ClientID);
          }

        } else if (match_command(cmd_is_iodev_connected, len)) {
          int UserID = 0;
          int DeviceID = 0;
          sscanf(&buffer[strnlen(cmd_is_iodev_connected, IPC_BUFFER_SIZE)],
                 "%i,%i", &UserID, &DeviceID);

          if (UserID && DeviceID &&
              supla_user::is_device_online(UserID, DeviceID)) {
            send_result("CONNECTED:", DeviceID);
          } else {
            send_result("DISCONNECTED:", DeviceID);
          }
        } else if (match_command(cmd_is_channel_connected, len)) {
          int UserID = 0;
          int DeviceID = 0;
          int ChannelID = 0;
          sscanf(&buffer[strnlen(cmd_is_channel_connected, IPC_BUFFER_SIZE)],
                 "%i,%i,%i", &UserID, &DeviceID, &ChannelID);

          if (UserID && DeviceID && ChannelID &&
              supla_user::is_channel_online(UserID, DeviceID, ChannelID)) {
            send_result("CONNECTED:", ChannelID);
          } else {
            send_result("DISCONNECTED:", ChannelID);
          }
        } else if (match_command(cmd_user_reconnect, len)) {
          int UserID = 0;
          sscanf(&buffer[strnlen(cmd_user_reconnect, IPC_BUFFER_SIZE)], "%i",
                 &UserID);

          if (UserID && supla_user::reconnect(UserID, supla_caller(ctIPC))) {
            send_result("OK:", UserID);
          } else {
            send_result("USER_UNKNOWN:", UserID);
          }

        } else if (match_command(cmd_client_reconnect, len)) {
          int UserID = 0;
          int ClientID = 0;

          sscanf(&buffer[strnlen(cmd_client_reconnect, IPC_BUFFER_SIZE)],
                 "%i,%i", &UserID, &ClientID);

          if (UserID && ClientID) {
            supla_user::client_reconnect(UserID, ClientID);
            send_result("OK:", ClientID);
          } else {
            send_result("USER_OR_CLIENT_UNKNOWN");
          }

        } else if (match_command(cmd_get_double_value, len)) {
          get_double(cmd_get_double_value, 0);

        } else if (match_command(cmd_get_temperature_value, len)) {
          get_double(cmd_get_temperature_value, 1);

        } else if (match_command(cmd_get_humidity_value, len)) {
          get_double(cmd_get_humidity_value, 2);

        } else if (match_command(cmd_get_rgbw_value, len)) {
          get_rgbw(cmd_get_rgbw_value);

        } else if (match_command(cmd_get_char_value, len)) {
          get_char(cmd_get_char_value);

        } else if (match_command(cmd_get_em_value, len)) {
          get_electricitymeter_value(cmd_get_em_value);

        } else if (match_command(cmd_get_ic_value, len)) {
          get_impulsecounter_value(cmd_get_ic_value);

        } else if (match_command(cmd_get_valve_value, len)) {
          get_valve_value(cmd_get_valve_value);

        } else if (match_command(cmd_get_digiglass_value, len)) {
          get_digiglass_value(cmd_get_digiglass_value);

        } else if (match_command(cmd_set_char_value, len)) {
          set_char(cmd_set_char_value, false);

        } else if (match_command(cmd_set_rgbw_value, len)) {
          set_rgbw(cmd_set_rgbw_value, false, false);

        } else if (match_command(cmd_set_rand_rgbw_value, len)) {
          set_rgbw(cmd_set_rand_rgbw_value, false, true);

        } else if (match_command(cmd_set_cg_char_value, len)) {
          set_char(cmd_set_cg_char_value, true);

        } else if (match_command(cmd_set_cg_rgbw_value, len)) {
          set_rgbw(cmd_set_cg_rgbw_value, true, false);

        } else if (match_command(cmd_set_cg_rand_rgbw_value, len)) {
          set_rgbw(cmd_set_cg_rand_rgbw_value, true, true);

        } else if (match_command(cmd_user_alexa_credentials_changed, len)) {
          alexa_credentials_changed(cmd_user_alexa_credentials_changed);

        } else if (match_command(cmd_user_google_home_credentials_changed,
                                 len)) {
          google_home_credentials_changed(
              cmd_user_google_home_credentials_changed);

        } else if (match_command(cmd_user_state_webhook_changed, len)) {
          state_webhook_changed(cmd_user_state_webhook_changed);

        } else if (match_command(cmd_user_mqtt_settings_changed, len)) {
          mqtt_settings_changed(cmd_user_mqtt_settings_changed);

        } else if (match_command(cmd_user_on_device_deleted, len)) {
          on_device_deleted(cmd_user_on_device_deleted);

        } else if (match_command(cmd_user_before_device_delete, len)) {
          before_device_delete(cmd_user_before_device_delete);

        } else if (match_command(cmd_user_before_channel_function_change,
                                 len)) {
          before_channel_function_change(
              cmd_user_before_channel_function_change);

        } else if (match_command(cmd_user_on_device_settings_changed, len)) {
          on_device_settings_changed(cmd_user_on_device_settings_changed);

        } else if (match_command(cmd_set_digiglass_value, len)) {
          set_digiglass_value(cmd_set_digiglass_value);
        } else if (match_command(cmd_action_open, len)) {
          action_open_close(cmd_action_open, true);
        } else if (match_command(cmd_action_cg_open, len)) {
          action_cg_open_close(cmd_action_cg_open, true);
        } else if (match_command(cmd_action_close, len)) {
          action_open_close(cmd_action_close, false);
        } else if (match_command(cmd_action_cg_close, len)) {
          action_cg_open_close(cmd_action_cg_close, false);
        } else if (match_command(cmd_action_toggle, len)) {
          action_toggle(cmd_action_toggle);
        } else if (match_command(cmd_action_cg_toggle, len)) {
          action_cg_toggle(cmd_action_cg_toggle);
        } else if (match_command(cmd_action_stop, len)) {
          action_stop(cmd_action_stop);
        } else if (match_command(cmd_action_cg_stop, len)) {
          action_cg_stop(cmd_action_cg_stop);
        } else if (match_command(cmd_action_up_or_stop, len)) {
          action_up_or_stop(cmd_action_up_or_stop);
        } else if (match_command(cmd_action_cg_up_or_stop, len)) {
          action_cg_up_or_stop(cmd_action_cg_up_or_stop);
        } else if (match_command(cmd_action_down_or_stop, len)) {
          action_down_or_stop(cmd_action_down_or_stop);
        } else if (match_command(cmd_action_cg_down_or_stop, len)) {
          action_cg_down_or_stop(cmd_action_cg_down_or_stop);
        } else if (match_command(cmd_action_step_by_step, len)) {
          action_step_by_step(cmd_action_step_by_step);
        } else if (match_command(cmd_action_cg_step_by_step, len)) {
          action_cg_step_by_step(cmd_action_cg_step_by_step);
        } else if (match_command(cmd_action_copy, len)) {
          action_copy(cmd_action_copy);
        } else if (match_command(cmd_action_cg_copy, len)) {
          action_cg_copy(cmd_action_cg_copy);
        } else if (match_command(cmd_get_relay_value, len)) {
          get_relay_value(cmd_get_relay_value);
        } else if (match_command(cmd_reset_counters, len)) {
          reset_counters(cmd_reset_counters);
        } else if (match_command(cmd_recalibrate, len)) {
          recalibrate(cmd_recalibrate);
        } else if (match_command(cmd_enter_cfg_mode, len)) {
          enter_cfg_mode(cmd_enter_cfg_mode);
        } else if (match_command(cmd_get_status, len)) {
          get_status();
        } else if (match_command(cmd_execute_scene, len)) {
          execute_scene(cmd_execute_scene);
        } else if (match_command(cmd_interrupt_scene, len)) {
          interrupt_scene(cmd_interrupt_scene);
        } else {
          supla_log(LOG_WARNING, "IPC - COMMAND UNKNOWN: %s", buffer);
          send_result("COMMAND_UNKNOWN");
        }

        gettimeofday(&last_action, NULL);
      }

    } else {
      sthread_terminate(sthread);
    }

    struct timeval now;
    gettimeofday(&now, NULL);

    if (now.tv_sec - last_action.tv_sec >= 5) {
      sthread_terminate(sthread);
      break;
    }
  }
}

supla_ipc_ctrl::~supla_ipc_ctrl() {
  if (sfd != -1) close(sfd);

  eh_free(eh);
}