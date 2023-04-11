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

#include "push_notification_recipient.h"

using std::string;

supla_push_notification_recipient::supla_push_notification_recipient(
    type_e type, const string& id) {
  this->type = type;
  this->id = id;
}

const std::string& supla_push_notification_recipient::get_id(void) {
  return id;
}

supla_push_notification_recipient::type_e
supla_push_notification_recipient::get_type(void) {
  return type;
}

supla_push_notification_recipient::~supla_push_notification_recipient(void) {}
