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

#include "ipc/abstract_device_set_time_command.h"

using std::string;

supla_abstract_device_set_time_command::supla_abstract_device_set_time_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_device_set_time_command::get_command_name(void) {
  return "DEVICE-SET-TIME:";
}

void supla_abstract_device_set_time_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int device_id = 0;
  int year = 0;
  int month = 0;
  int day = 0;
  int day_of_week = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  supla_user *user = NULL;

  char timezone[100] = {};

  if (params) {
    sscanf(params, "%i,%i,%i,%i,%i,%i,%i,%i,%i,%99s", &user_id, &device_id,
           &year, &month, &day, &day_of_week, &hour, &min, &sec, timezone);

    if (user_id && device_id && year >= 1900 && year <= 3000 && month >= 1 &&
        month <= 12 && day >= 1 && day <= 31 && day_of_week >= 1 &&
        day_of_week <= 7 && hour >= 0 && hour <= 23 && min >= 0 && min <= 59 &&
        sec >= 0 && sec <= 59 &&
        (user = supla_user::find(user_id, false)) != nullptr) {
      char *tz = st_openssl_base64_decode(
          timezone, strnlen(timezone, sizeof(timezone)), nullptr);

      *timezone = {};

      if (set_time(user, device_id, year, month, day, day_of_week, hour, min,
                   sec, tz ? tz : timezone)) {
        send_result("OK:", device_id);
      } else {
        send_result("FAIL:", device_id);
      }

      if (tz) {
        free(tz);
      }

      return;
    }
  }
  send_result("UNKNOWN:", user_id);
}
