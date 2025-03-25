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

#include "channel_availability_status.h"

#include "proto.h"

supla_channel_availability_status::supla_channel_availability_status(void) {
  proto_offline = SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE;
}

supla_channel_availability_status::supla_channel_availability_status(
    bool offline) {
  proto_offline = offline ? SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE
                          : SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE;
}

supla_channel_availability_status::supla_channel_availability_status(
    char status, bool proto_offline) {
  this->proto_offline = 0;
  if (proto_offline) {
    set_proto_offline(status);
  } else {
    set_proto_online(status);
  }
}

supla_channel_availability_status::~supla_channel_availability_status(void) {}

bool supla_channel_availability_status::is_online(void) const {
  return proto_offline == SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE;
}

bool supla_channel_availability_status::is_offline(void) const {
  return proto_offline == SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE;
}

bool supla_channel_availability_status::is_online_but_not_available(
    void) const {
  return proto_offline == SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE_BUT_NOT_AVAILABLE;
}

bool supla_channel_availability_status::is_offline_remote_wakeup_not_supported(
    void) const {
  return proto_offline ==
         SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE_REMOTE_WAKEUP_NOT_SUPPORTED;
}

bool supla_channel_availability_status::is_firmware_update_ongoing(void) const {
  return proto_offline == SUPLA_CHANNEL_OFFLINE_FLAG_FIRMWARE_UPDATE_ONGOING;
}

char supla_channel_availability_status::get_proto_online(void) const {
  switch (proto_offline) {
    case SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE_BUT_NOT_AVAILABLE:
    case SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE_REMOTE_WAKEUP_NOT_SUPPORTED:
    case SUPLA_CHANNEL_OFFLINE_FLAG_FIRMWARE_UPDATE_ONGOING:
      return proto_offline;
  }

  return !proto_offline;
}

char supla_channel_availability_status::get_proto_offline(void) const {
  return proto_offline;
}

void supla_channel_availability_status::set_proto_online(char online) {
  switch (online) {
    case SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE_BUT_NOT_AVAILABLE:
    case SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE_REMOTE_WAKEUP_NOT_SUPPORTED:
    case SUPLA_CHANNEL_OFFLINE_FLAG_FIRMWARE_UPDATE_ONGOING:
      proto_offline = online;
      break;
    default:
      proto_offline = !online;
      break;
  }
}

void supla_channel_availability_status::set_proto_offline(char offline) {
  switch (offline) {
    case SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE_BUT_NOT_AVAILABLE:
    case SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE_REMOTE_WAKEUP_NOT_SUPPORTED:
    case SUPLA_CHANNEL_OFFLINE_FLAG_FIRMWARE_UPDATE_ONGOING:
      proto_offline = offline;
      break;
    default:
      proto_offline = !!offline;
      break;
  }
}

void supla_channel_availability_status::set_offline(bool offline) {
  proto_offline = offline ? SUPLA_CHANNEL_OFFLINE_FLAG_OFFLINE
                          : SUPLA_CHANNEL_OFFLINE_FLAG_ONLINE;
}

bool supla_channel_availability_status::operator==(
    const supla_channel_availability_status &status) const {
  return this->proto_offline == status.proto_offline;
}

bool supla_channel_availability_status::operator!=(
    const supla_channel_availability_status &status) const {
  return this->proto_offline != status.proto_offline;
}
