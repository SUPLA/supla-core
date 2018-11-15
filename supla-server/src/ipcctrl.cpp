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
#include "ipcctrl.h"
#include "ipcsocket.h"
#include "log.h"
#include "sthread.h"
#include "tools.h"
#include "user.h"

const char hello[] = "SUPLA SERVER CTRL\n";
const char cmd_is_client_connected[] = "IS-CLIENT-CONNECTED:";
const char cmd_is_iodev_connected[] = "IS-IODEV-CONNECTED:";
const char cmd_user_reconnect[] = "USER-RECONNECT:";
const char cmd_client_reconnect[] = "CLIENT-RECONNECT:";
const char cmd_get_double_value[] = "GET-DOUBLE-VALUE:";
const char cmd_get_temperature_value[] = "GET-TEMPERATURE-VALUE:";
const char cmd_get_humidity_value[] = "GET-HUMIDITY-VALUE:";
const char cmd_get_char_value[] = "GET-CHAR-VALUE:";
const char cmd_get_rgbw_value[] = "GET-RGBW-VALUE:";

const char cmd_set_char_value[] = "SET-CHAR-VALUE:";
const char cmd_set_rgbw_value[] = "SET-RGBW-VALUE:";
const char cmd_set_rand_rgbw_value[] = "SET-RAND-RGBW-VALUE:";

const char cmd_set_cg_char_value[] = "SET-CG-CHAR-VALUE:";
const char cmd_set_cg_rgbw_value[] = "SET-CG-RGBW-VALUE:";
const char cmd_set_cg_rand_rgbw_value[] = "SET-CG-RAND-RGBW-VALUE:";

const char cmd_user_alexa_egc_changed[] = "USER-ALEXA-EGC-CHANGED:";

char ACT_VAR[] = ",ALEXA-CORRELATION-TOKEN=";

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

  sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r = supla_user::get_channel_rgbw_value(
        UserID, DeviceID, ChannelID, &color, &color_brightness, &brightness);

    if (r) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i,%i\n", color,
               color_brightness, brightness);
      send(sfd, buffer, strnlen(buffer, IPC_BUFFER_SIZE), 0);

      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void svr_ipcctrl::free_correlation_token() {
  if (AlexaCorelationToken) {
    free(AlexaCorelationToken);
    AlexaCorelationToken = NULL;
  }
}

void svr_ipcctrl::cut_correlation_token(const char *cmd) {
  free_correlation_token();

  char *ct = strstr(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], ACT_VAR);

  if (ct != NULL && strnlen(ct, IPC_BUFFER_SIZE) > sizeof(ACT_VAR) - 1) {
    char *value = &ct[sizeof(ACT_VAR) - 1];

    int len = strnlen(value, IPC_BUFFER_SIZE);

    if (len > 0) {
      AlexaCorelationToken = st_openssl_base64_decode(value, len, NULL);
    }

    ct[0] = 0;
  }
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
          UserID, 0, DeviceID, CGID, Value, AlexaCorelationToken);
    }

    free_correlation_token();

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
      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i", &UserID,
             &CGID, &Color, &ColorBrightness, &Brightness);

    } else {
      cut_correlation_token(cmd);

      sscanf(&buffer[strnlen(cmd, IPC_BUFFER_SIZE)], "%i,%i,%i,%i,%i,%i",
             &UserID, &DeviceID, &CGID, &Color, &ColorBrightness, &Brightness);
    }
  }

  if (UserID && CGID) {
    if (ColorBrightness < 0 || ColorBrightness > 100 || Brightness < 0 ||
        Brightness > 100)
      send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = supla_user::set_channelgroup_rgbw_value(
          UserID, CGID, Color, ColorBrightness, Brightness);
    } else if (!group && DeviceID) {
      result = supla_user::set_device_channel_rgbw_value(
          UserID, 0, DeviceID, CGID, Color, ColorBrightness, Brightness,
          AlexaCorelationToken);
    }

    free_correlation_token();

    if (result) {
      send_result("OK:", CGID);
    } else {
      send_result("FAIL:", CGID);
    }

    return;
  }

  send_result("UNKNOWN:", CGID);
}

void svr_ipcctrl::alexa_egc_changed(const char *cmd) {
  int UserID = 0;

  sscanf(&buffer[strnlen(cmd_user_alexa_egc_changed, IPC_BUFFER_SIZE)], "%i",
         &UserID);

  if (UserID) {
    supla_user::on_alexa_egc_changed(UserID);
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
        supla_log(LOG_DEBUG, "%s", buffer);

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

          if (UserID && supla_user::reconnect(UserID)) {
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

        } else if (match_command(cmd_user_alexa_egc_changed, len)) {
          alexa_egc_changed(cmd_user_alexa_egc_changed);

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
