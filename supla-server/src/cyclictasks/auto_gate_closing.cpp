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
#include "device/value_getter.h"

using std::vector;

supla_auto_gate_closing::supla_auto_gate_closing(
    supla_abstract_auto_gate_closing_dao *dao,
    supla_abstract_value_getter *value_getter)
    : supla_abstract_cyclictask() {
  this->dao = dao;
  this->value_getter = value_getter;
}

supla_auto_gate_closing::supla_auto_gate_closing()
    : supla_abstract_cyclictask() {
  this->dao = nullptr;
  this->value_getter = nullptr;
}

supla_auto_gate_closing::~supla_auto_gate_closing() {}

bool supla_auto_gate_closing::user_access_needed(void) { return false; }

unsigned int supla_auto_gate_closing::task_interval_sec(void) { return 60; }

_gate_sensor_level_enum supla_auto_gate_closing::get_opening_sensor_level(
    supla_abstract_auto_gate_closing_dao::item_t *item) {
  _gate_sensor_level_enum opening_sensor_level = gsl_unknown;

  supla_abstract_value_getter *internal_value_getter = nullptr;

  if (value_getter == nullptr) {
    internal_value_getter = new supla_value_getter();
    value_getter = internal_value_getter;
  }

  supla_channel_value *value =
      value_getter->get_value(item->user_id, item->device_id, item->channel_id);

  if (internal_value_getter) {
    value_getter = nullptr;
    delete internal_value_getter;
  }

  if (value) {
    supla_channel_gate_value *gate_value =
        dynamic_cast<supla_channel_gate_value *>(value);
    if (gate_value) {
      opening_sensor_level = gate_value->get_opening_sensor_level();
    }
    delete value;
    value = nullptr;
  }

  return opening_sensor_level;
}

void supla_auto_gate_closing::run(const vector<supla_user *> *users,
                                  supla_abstract_db_access_provider *dba) {
  if (!dba->connect()) {
    return;
  }

  supla_abstract_auto_gate_closing_dao *internal_dao = nullptr;
  if (dao == nullptr) {
    internal_dao = new supla_auto_gate_closing_dao(dba);
    dao = internal_dao;
  }

  std::vector<supla_abstract_auto_gate_closing_dao::item_t> items =
      dao->get_all_active();

  for (auto it = items.begin(); it != items.end(); ++it) {
    _gate_sensor_level_enum level = get_opening_sensor_level(&(*it));
    if (level == gsl_unknown) {
      continue;
    }
    if (level == gsl_closed) {
      if (it->seen_open) {
        dao->mark_gate_closed(it->channel_id);
      }
    } else if (level == gsl_open) {
      int time_left = dao->mark_gate_open(it->channel_id);
      if (time_left <= 0 &&
          it->seconds_since_last_attempt >= ATTEMPT_RETRY_TIME_SEC) {
        dao->set_closing_attemtp(it->channel_id);
        supla_action_gate_openclose::open_close(supla_caller(ctCyclicTask),
                                                it->user_id, it->device_id,
                                                it->channel_id, false);
      }
    }
  }

  if (internal_dao) {
    delete internal_dao;
    dao = nullptr;
  }
}
