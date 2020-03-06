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
#include "devicecontainer.h"
#include "device/device.h"

// static
char supla_user_device_container::find_device_byid(void *ptr, void *ID) {
  return ((supla_device *)ptr)->getID() == *(int *)ID ? 1 : 0;
}

// static
char supla_user_device_container::find_device_by_channelid(void *ptr,
                                                           void *ID) {
  return ((supla_device *)ptr)->channel_exists(*(int *)ID) ? 1 : 0;
}

// static
char supla_user_device_container::find_device_byguid(void *ptr, void *GUID) {
  return ((supla_device *)ptr)->cmpGUID((char *)GUID) ? 1 : 0;
}

supla_user_device_container::supla_user_device_container() : cdcontainer() {}

supla_user_device_container::~supla_user_device_container() {}

void supla_user_device_container::cd_delete(cdbase *base) {
  supla_device *device = static_cast<supla_device *>(base);
  if (device) {
    delete device;
  }
}

supla_device *supla_user_device_container::baseToDevice(cdbase *base) {
  supla_device *device = NULL;
  if (base && (device = static_cast<supla_device *>(base)) == NULL) {
    base->releasePtr();
  }
  return device;
}

supla_device *supla_user_device_container::findByID(int DeviceID) {
  if (DeviceID == 0) {
    return NULL;
  }
  return baseToDevice(find(find_device_byid, &DeviceID));
}

supla_device *supla_user_device_container::findByChannelID(int ChannelID) {
  if (ChannelID == 0) {
    return NULL;
  }
  return baseToDevice(find(find_device_by_channelid, &ChannelID));
}

supla_device *supla_user_device_container::findByGUID(const char *GUID) {
  return baseToDevice(find(find_device_byguid, (void *)GUID));
}

supla_device *supla_user_device_container::get(int idx) {
  return static_cast<supla_device *>(cdcontainer::get(idx));
}
