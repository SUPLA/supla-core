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
#include <unistd.h>

#include "database.h"
#include "ipcctrl.h"
#include "ipcsocket.h"
#include "log.h"
#include "sthread.h"
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

const char cmd_oauth[] = "OAUTH:";
const char cmd_sauth[] = "SAUTH:";  // SUPER USER AUTH
const char cmd_set_char_value[] = "SET-CHAR-VALUE:";
const char cmd_set_rgbw_value[] = "SET-RGBW-VALUE:";

const char cmd_set_cg_char_value[] = "SET-CG-CHAR-VALUE:";
const char cmd_set_cg_rgbw_value[] = "SET-CG-RGBW-VALUE:";

svr_ipcctrl::svr_ipcctrl(int sfd) {
  set_unauthorized();

  this->sfd = sfd;

  this->eh = eh_init();
  eh_add_fd(eh, sfd);
}

bool svr_ipcctrl::match_command(const char *cmd, int len) {
  if (len > (int)strnlen(cmd, 255) &&
      memcmp(buffer, cmd, strnlen(cmd, 255)) == 0 && buffer[len - 1] == '\n') {
    buffer[len - 1] = 0;
    return true;
  }

  return false;
}

void svr_ipcctrl::send_result(const char *result) {
  snprintf(buffer, sizeof(buffer), "%s\n", result);
  send(sfd, buffer, strnlen(buffer, 255), 0);
}

void svr_ipcctrl::send_result(const char *result, int i) {
  snprintf(buffer, sizeof(buffer), "%s%i\n", result, i);
  send(sfd, buffer, strnlen(buffer, 255), 0);
}

void svr_ipcctrl::send_result(const char *result, double i) {
  snprintf(buffer, sizeof(buffer), "%s%f\n", result, i);
  send(sfd, buffer, strnlen(buffer, 255), 0);
}

void svr_ipcctrl::get_double(const char *cmd, char Type) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;
  double Value;

  sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i", &UserID, &DeviceID,
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

  sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i", &UserID, &DeviceID,
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

void svr_ipcctrl::set_unauthorized(void) {
  auth_level = IPC_AUTH_LEVEL_UNAUTHORIZED;
  oauth_user_id = 0;
  user_id = 0;
  auth_expires_at = 0;
}

void svr_ipcctrl::oauth(const char *cmd) {
  set_unauthorized();

  int _auth_expires_at = 0;
  int _user_id = 0;
  int _oauth_user_id = 0;

  bool result = false;

  char access_token[256];
  memset(access_token, 0, 256);

  sscanf(&buffer[strnlen(cmd, 255)], "%s\n", access_token);
  access_token[255] = 0;

  database *db = new database();

  if (db->connect() == true) {
    if (db->get_oauth_user(access_token, &_oauth_user_id, &_user_id,
                           &_auth_expires_at) &&
        _user_id > 0) {
      oauth_user_id = _oauth_user_id;
      user_id = _user_id;
      auth_expires_at = _auth_expires_at;
      auth_level = IPC_AUTH_LEVEL_OAUTH_USER;

      result = true;
    }
  }

  delete db;

  if (result) {
    send_result("AUTH_OK:", user_id);
    return;
  }

  send_result("UNAUTHORIZED");
}

void svr_ipcctrl::sauth(const char *cmd) {
  set_unauthorized();

  if (ipc_sauth_key != NULL &&
      memcmp(&buffer[strnlen(cmd, 255)], ipc_sauth_key, IPC_SAUTH_KEY_SIZE) ==
          0) {
    auth_level = IPC_AUTH_LEVEL_SUPERUSER;

    send_result("AUTH_OK");
    return;
  }

  send_result("UNAUTHORIZED");
}

bool svr_ipcctrl::is_authorized(int UserID, bool _send_result) {
  bool result = auth_level == IPC_AUTH_LEVEL_SUPERUSER ||
                (auth_level == IPC_AUTH_LEVEL_OAUTH_USER && user_id == UserID);

  if (result && auth_expires_at > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);

    if (auth_expires_at < now.tv_sec) result = false;
  }

  if (result == false) {
    send_result("UNAUTHORIZED");
  }

  return result;
}

void svr_ipcctrl::get_rgbw(const char *cmd) {
  int UserID = 0;
  int DeviceID = 0;
  int ChannelID = 0;

  int color;
  char color_brightness;
  char brightness;

  sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i", &UserID, &DeviceID,
         &ChannelID);

  if (UserID && DeviceID && ChannelID) {
    bool r = supla_user::get_channel_rgbw_value(
        UserID, DeviceID, ChannelID, &color, &color_brightness, &brightness);

    if (r) {
      snprintf(buffer, sizeof(buffer), "VALUE:%i,%i,%i\n", color,
               color_brightness, brightness);
      send(sfd, buffer, strnlen(buffer, 255), 0);

      return;
    }
  }

  send_result("UNKNOWN:", ChannelID);
}

void svr_ipcctrl::set_char(const char *cmd, bool group) {
  int UserID = 0;
  int CGID = 0;
  int DeviceID = 0;
  int Value = 0;

  if (group) {
    sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i", &UserID, &CGID, &Value);
  } else {
    sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i,%i", &UserID, &DeviceID, &CGID,
           &Value);
  }

  if (UserID && CGID) {
    if (false == is_authorized(UserID, true)) return;

    if (Value < 0 || Value > 255) send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = supla_user::set_channelgroup_char_value(UserID, CGID, Value);
    } else if (!group && DeviceID) {
      result = supla_user::set_device_channel_char_value(UserID, 0, DeviceID,
                                                         CGID, Value);
    }

    if (result) {
      send_result("OK:", CGID);
    } else {
      send_result("FAIL:", CGID);
    }

    return;
  }

  send_result("UNKNOWN:", CGID);
}

void svr_ipcctrl::set_rgbw(const char *cmd, bool group) {
  int UserID = 0;
  int DeviceID = 0;
  int CGID = 0;
  int Color = 0;
  int ColorBrightness = 0;
  int Brightness = 0;

  if (group) {
    sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i,%i,%i", &UserID, &CGID, &Color,
           &ColorBrightness, &Brightness);

  } else {
    sscanf(&buffer[strnlen(cmd, 255)], "%i,%i,%i,%i,%i,%i", &UserID, &DeviceID,
           &CGID, &Color, &ColorBrightness, &Brightness);
  }

  if (UserID && CGID) {
    if (false == is_authorized(UserID, true)) return;

    if (ColorBrightness < 0 || ColorBrightness > 100 || Brightness < 0 ||
        Brightness > 100)
      send_result("VALUE OUT OF RANGE");

    bool result = false;

    if (group) {
      result = supla_user::set_channelgroup_rgbw_value(
          UserID, CGID, Color, ColorBrightness, Brightness);
    } else if (!group && DeviceID) {
      result = supla_user::set_device_channel_rgbw_value(
          UserID, 0, DeviceID, CGID, Color, ColorBrightness, Brightness);
    }

    if (result) {
      send_result("OK:", CGID);
    } else {
      send_result("FAIL:", CGID);
    }

    return;
  }

  send_result("UNKNOWN:", CGID);
}

void svr_ipcctrl::execute(void *sthread) {
  if (sfd == -1) return;

  int len;

  struct timeval last_action;
  gettimeofday(&last_action, NULL);

  send(sfd, hello, sizeof(hello), 0);

  while (sthread_isterminated(sthread) == 0) {
    eh_wait(eh, 1000000);
    memset(buffer, 0, 255);

    if ((len = recv(sfd, buffer, sizeof(buffer), 0)) != 0) {
      if (len > 0) {
        buffer[255] = 0;

        if (match_command(cmd_is_client_connected, len)) {
          int UserID = 0;
          int ClientID = 0;
          sscanf(&buffer[strnlen(cmd_is_client_connected, 255)], "%i,%i",
                 &UserID, &ClientID);

          if (UserID && ClientID &&
              supla_user::is_client_online(UserID, ClientID)) {
            send_result("CONNECTED:", ClientID);
          } else {
            send_result("DISCONNECTED:", ClientID);
          }

        } else if (match_command(cmd_is_iodev_connected, len)) {
          int UserID = 0;
          int DeviceID = 0;
          sscanf(&buffer[strnlen(cmd_is_iodev_connected, 255)], "%i,%i",
                 &UserID, &DeviceID);

          if (UserID && DeviceID &&
              supla_user::is_device_online(UserID, DeviceID)) {
            send_result("CONNECTED:", DeviceID);
          } else {
            send_result("DISCONNECTED:", DeviceID);
          }
        } else if (match_command(cmd_user_reconnect, len)) {
          int UserID = 0;
          sscanf(&buffer[strnlen(cmd_user_reconnect, 255)], "%i", &UserID);

          if (UserID && supla_user::reconnect(UserID)) {
            send_result("OK:", UserID);
          } else {
            send_result("USER_UNKNOWN:", UserID);
          }

        } else if (match_command(cmd_client_reconnect, len)) {
          int UserID = 0;
          int ClientID = 0;

          sscanf(&buffer[strnlen(cmd_client_reconnect, 255)], "%i,%i", &UserID,
                 &ClientID);

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

        } else if (match_command(cmd_oauth, len)) {
          oauth(cmd_oauth);

        } else if (match_command(cmd_sauth, len)) {
          sauth(cmd_sauth);

        } else if (match_command(cmd_set_char_value, len)) {
          set_char(cmd_set_char_value, false);

        } else if (match_command(cmd_set_rgbw_value, len)) {
          set_rgbw(cmd_set_rgbw_value, false);

        } else if (match_command(cmd_set_cg_char_value, len)) {
          set_char(cmd_set_cg_char_value, true);

        } else if (match_command(cmd_set_rgbw_value, len)) {
          set_rgbw(cmd_set_cg_rgbw_value, true);

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
