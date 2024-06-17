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

#include "device_set_time_command.h"

#include <memory>

#include "device.h"
#include "http/http_event_hub.h"
#include "user.h"

using std::shared_ptr;

supla_device_set_time_command::supla_device_set_time_command(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_device_set_time_command(socket_adapter) {}

bool supla_device_set_time_command::set_time(
    supla_user *user, int device_id, unsigned _supla_int16_t year,
    unsigned char month, unsigned char day, unsigned char day_of_week,
    unsigned char hour, unsigned char min, unsigned char sec,
    const char *timezone) {
  shared_ptr<supla_device> device = user->get_devices()->get(device_id, 0);

  if (device != nullptr) {
    TSDC_UserLocalTimeResult result = {};
    result.year = year;
    result.month = month;
    result.day = day;
    result.dayOfWeek = day_of_week;
    result.hour = hour;
    result.min = min;
    result.sec = sec;

    snprintf(result.timezone, sizeof(result.timezone), "%s", timezone);
    result.timezoneSize = strnlen(result.timezone, sizeof(result.timezone));

    return device->get_connection()
        ->get_srpc_adapter()
        ->sdc_async_get_user_localtime_result(&result);
  }

  return false;
}
