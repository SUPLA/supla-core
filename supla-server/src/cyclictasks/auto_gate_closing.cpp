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

#include "cyclictasks/auto_gate_closing.h"

#include <assert.h>

#include "actions/action_gate_openclose.h"
#include "cyclictasks/auto_gate_closing_dao.h"
#include "device/channel_property_getter.h"

using std::vector;

supla_auto_gate_closing::supla_auto_gate_closing()
    : supla_abstract_auto_gate_closing() {}

supla_auto_gate_closing::~supla_auto_gate_closing() {}

supla_abstract_auto_gate_closing_dao *supla_auto_gate_closing::get_dao(
    supla_abstract_db_access_provider *dba) {
  return new supla_auto_gate_closing_dao(dba);
}

void supla_auto_gate_closing::release_dao(
    supla_abstract_auto_gate_closing_dao *dao) {
  delete dao;
}

supla_abstract_channel_property_getter *
supla_auto_gate_closing::get_property_getter(void) {
  return new supla_cahnnel_property_getter();
}

void supla_auto_gate_closing::release_property_getter(
    supla_abstract_channel_property_getter *property_getter) {
  delete property_getter;
}

void supla_auto_gate_closing::close_the_gate(int user_id, int device_id,
                                             int channel_id) {
  supla_action_gate_openclose::open_close(supla_caller(ctCyclicTask), user_id,
                                          device_id, channel_id, false);
}
