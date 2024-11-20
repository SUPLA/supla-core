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

channel_availability_status::channel_availability_status(void) {
  proto_offline = 0;
}

channel_availability_status::channel_availability_status(char status,
                                                         bool proto_offline) {
  this->proto_offline = 0;
  if (proto_offline) {
    set_proto_offline(status);
  } else {
    set_proto_online(status);
  }
}

channel_availability_status::~channel_availability_status(void) {}

bool channel_availability_status::is_online(void) const {
  return proto_offline == 0;
}

bool channel_availability_status::is_offline(void) const {
  return proto_offline == 1;
}

bool channel_availability_status::is_online_but_not_available(void) const {
  return proto_offline == 2;
}

char channel_availability_status::get_proto_online(void) const {
  if (proto_offline == 2) {
    return proto_offline;
  }

  return !proto_offline;
}

char channel_availability_status::get_proto_offline(void) const {
  return proto_offline;
}

void channel_availability_status::set_proto_online(char online) {
  if (online == 2) {
    proto_offline = 2;
  } else {
    proto_offline = !online;
  }
}

void channel_availability_status::set_proto_offline(char offline) {
  if (offline == 2) {
    proto_offline = 2;
  } else {
    proto_offline = !!offline;
  }
}

bool channel_availability_status::operator==(
    const channel_availability_status &status) const {
  return this->proto_offline == status.proto_offline;
}

bool channel_availability_status::operator!=(
    const channel_availability_status &status) const {
  return this->proto_offline != status.proto_offline;
}
