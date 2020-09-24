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
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "database.h"
#include "http/httprequestqueue.h"
#include "ipcctrl.h"
#include "ipcsocket.h"
#include "log.h"
#include "sthread.h"
#include "tools.h"
#include "user.h"

const char hello[] = "SUPLA SERVER CTRL\n";
const char cmd_is_client_connected[] = "IS-CLIENT-CONNECTED:";
const char cmd_is_iodev_connected[] = "IS-IODEV-CONNECTED:";
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

const char cmd_set_char_value[] = "SET-CHAR-VALUE:";
const char cmd_set_rgbw_value[] = "SET-RGBW-VALUE:";
const char cmd_set_rand_rgbw_value[] = "SET-RAND-RGBW-VALUE:";

const char cmd_set_cg_char_value[] = "SET-CG-CHAR-VALUE:";
const char cmd_set_cg_rgbw_value[] = "SET-CG-RGBW-VALUE:";
const char cmd_set_cg_rand_rgbw_value[] = "SET-CG-RAND-RGBW-VALUE:";

const char cmd_user_alexa_credentials_changed[] =
    "USER-ALEXA-CREDENTIALS-CHANGED:";

const char cmd_user_google_home_credentials_changed[] =
    "USER-GOOGLE-HOME-CREDENTIALS-CHANGED:";

const char cmd_user_state_webhook_changed[] = "USER-STATE-WEBHOOK-CHANGED:";

const char cmd_user_on_device_deleted[] = "USER-ON-DEVICE-DELETED:";

char ACT_VAR[] = ",ALEXA-CORRELATION-TOKEN=";
char GRI_VAR[] = ",GOOGLE-REQUEST-ID=";

svr_ipcctrl::svr_ipcctrl(int sfd) {
  this->sfd = sfd;

  this->eh = eh_init();
  eh_add_fd(eh, sfd);
}

bool svr_ipcctrl::match_command(const char *cmd, int len) {
  if (len > (int)strnlen(cmd, IPC_BUFFER_SIZE) &&
      memcmp(buffer, cmd, strnlen(cmd, IPC_BUFFER_SIZE)) == 0 &&
      buffer[len - 1] == '\n') {
    buffer[len - 1] = 0;
    return true;
  }

  return false;
}

void svr_ipcctrl::send_result(const char *result) {
  snprintf(buffer, sizeof(buffer), "%s\n", result);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void svr_ipcctrl::send_result(const char *result, int i) {
  snprintf(buffer, sizeof(buffer), "%s%i\n", result, i);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void svr_ipcctrl::send_result(const char *result, double i) {
  snprintf(buffer, sizeof(buffer), "%s%f\n", result, i);
  send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);
}

void svr_ipcctrl::get_double(const char *cmd, char Type) {
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

void svr_ipcctrl::get_char(const char *cmd) {
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

void svr_ipcctrl::get_rgbw(const char *cmd) {
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

void svr_ipcctrl::get_impulsecounter_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    supla_channel_ic_measurement *icm =
        supla_user::get_ic_measurement(UserID, DeviceID, ChannelID);

    if (icm != NULL) {
      char *unit_b64 = st_openssl_base64_encode(
          (char *)icm->getCustomUnit(), strnlen(icm->getCustomUnit(), 9));

      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i,%i,%llu,%lld,%s,%s\n",
               icm->getTotalCost(), icm->getPricePerUnit(),
               icm->getImpulsesPerUnit(), icm->getCounter(),
               icm->getCalculatedValue(), icm->getCurrncy(),
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

void svr_ipcctrl::get_electricitymeter_value(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    supla_channel_electricity_measurement *em =
        supla_user::get_electricity_measurement(UserID, DeviceID, ChannelID);

    if (em != NULL) {
      TElectricityMeter_ExtendedValue em_ev;
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

      snprintf(buffer, sizeof(buffer),
               "VALUE:%i,%i,%i,%i,%i,%u,%u,%u,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,"
               "%i,%i,%i,%i,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%"
               "llu,%llu,%llu,%i,%i,%s\n",
               em_ev.measured_values, em_ev.m[0].freq, em_ev.m[0].voltage[0],
               em_ev.m[0].voltage[1], em_ev.m[0].voltage[2], current1, current2,
               current3, em_ev.m[0].power_active[0], em_ev.m[0].power_active[1],
               em_ev.m[0].power_active[2], em_ev.m[0].power_reactive[0],
               em_ev.m[0].power_reactive[1], em_ev.m[0].power_reactive[2],
               em_ev.m[0].power_apparent[0], em_ev.m[0].power_apparent[1],
               em_ev.m[0].power_apparent[2], em_ev.m[0].power_factor[0],
               em_ev.m[0].power_factor[1], em_ev.m[0].power_factor[2],
               em_ev.m[0].phase_angle[0], em_ev.m[0].phase_angle[1],
               em_ev.m[0].phase_angle[2], em_ev.total_forward_active_energy[0],
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

void svr_ipcctrl::get_valve_value(const char *cmd) {
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

void svr_ipcctrl::free_correlation_token() {
  if (AlexaCorrelationToken) {
    free(AlexaCorrelationToken);
    AlexaCorrelationToken = NULL;
  }
}

void svr_ipcctrl::free_google_requestid() {
  if (GoogleRequestId) {
    free(GoogleRequestId);
    GoogleRequestId = NULL;
  }
}

char *svr_ipcctrl::cut(const char *cmd, const char *var) {
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

void svr_ipcctrl::cut_correlation_token(const char *cmd) {
  free_correlation_token();
  AlexaCorrelationToken = cut(cmd, ACT_VAR);
}

void svr_ipcctrl::cut_google_requestid(const char *cmd) {
  free_google_requestid();
  GoogleRequestId = cut(cmd, GRI_VAR);
}

void svr_ipcctrl::set_char(const char *cmd, bool group) {
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

  if (UserID && CGID) {
    if (Value < 0 || Value > IPC_BUFFER_SIZE) send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = supla_user::set_channelgroup_char_value(UserID, CGID, Value);
    } else if (!group && DeviceID) {
      result = supla_user::set_device_channel_char_value(
          UserID, 0, DeviceID, CGID, 0, false, Value,
          AlexaCorrelationToken ? EST_AMAZON_ALEXA
                                : (GoogleRequestId ? EST_GOOGLE_HOME : EST_IPC),
          AlexaCorrelationToken, GoogleRequestId);
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

void svr_ipcctrl::set_rgbw(const char *cmd, bool group, bool random) {
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

    unsigned int seed = time(NULL);
    Color = st_hue2rgb(rand_r(&seed) % 360);

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

  if (UserID && CGID) {
    if (ColorBrightness < 0 || ColorBrightness > 100 || Brightness < 0 ||
        Brightness > 100)
      send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = supla_user::set_channelgroup_rgbw_value(
          UserID, CGID, Color, ColorBrightness, Brightness, TurnOnOff);
    } else if (!group && DeviceID) {
      result = supla_user::set_device_channel_rgbw_value(
          UserID, 0, DeviceID, CGID, 0, false, Color, ColorBrightness,
          Brightness, TurnOnOff,
          AlexaCorrelationToken ? EST_AMAZON_ALEXA
                                : (GoogleRequestId ? EST_GOOGLE_HOME : EST_IPC),
          AlexaCorrelationToken, GoogleRequestId);
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

void svr_ipcctrl::alexa_credentials_changed(const char *cmd) {
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

void svr_ipcctrl::google_home_credentials_changed(const char *cmd) {
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

void svr_ipcctrl::state_webhook_changed(const char *cmd) {
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

void svr_ipcctrl::on_device_deleted(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_on_device_deleted, IPC_BUFFER_SIZE)], "%i",
         &UserID);
  if (UserID) {
    supla_user::on_device_deleted(UserID, EST_IPC);
    send_result("OK:", UserID);
  } else {
    send_result("USER_UNKNOWN");
  }
}

void svr_ipcctrl::execute(void *sthread) {
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
        } else if (match_command(cmd_user_reconnect, len)) {
          int UserID = 0;
          sscanf(&buffer[strnlen(cmd_user_reconnect, IPC_BUFFER_SIZE)], "%i",
                 &UserID);

          if (UserID && supla_user::reconnect(UserID, EST_IPC)) {
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

        } else if (match_command(cmd_user_on_device_deleted, len)) {
          on_device_deleted(cmd_user_on_device_deleted);

        } else {
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

svr_ipcctrl::~svr_ipcctrl() {
  if (sfd != -1) close(sfd);

  eh_free(eh);
}
