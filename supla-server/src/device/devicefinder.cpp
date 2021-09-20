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

#include "device/devicefinder.h"

#include "device/device.h"
#include "user.h"

supla_device_finder::supla_device_finder(void)
    : supla_abstract_device_finder() {}
supla_device_finder::~supla_device_finder(void) {}

int supla_device_finder::find_device_id(int userId, int channelId) {
  supla_user *user = supla_user::find(userId, false);
  int result = 0;
  if (user) {
    supla_device *device = user->device_by_channelid(channelId);
    if (device) {
      result = device->getID();
      device->releasePtr();
    }
  }

  return result;
}
