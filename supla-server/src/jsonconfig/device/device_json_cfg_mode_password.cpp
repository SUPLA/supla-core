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

#include "device_json_cfg_mode_password.h"

#include <string.h>

#include "log.h"

#define FIELD_SET_CFG_MODE_PASSWORD 1

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string>
    device_json_cfg_mode_password::field_map = {
        {FIELD_SET_CFG_MODE_PASSWORD, "setCfgModePassword"}};

device_json_cfg_mode_password::device_json_cfg_mode_password(void)
    : device_json_async_result() {}

device_json_cfg_mode_password::device_json_cfg_mode_password(
    supla_json_config *root)
    : device_json_async_result(root) {}

device_json_cfg_mode_password::~device_json_cfg_mode_password(void) {}

string device_json_cfg_mode_password::get_status_root_key(void) {
  return field_map.at(FIELD_SET_CFG_MODE_PASSWORD);
}

const map<unsigned _supla_int16_t, string> &
device_json_cfg_mode_password::get_field_map(void) {
  return field_map;
}

void device_json_cfg_mode_password::set_true(void) {
  apply_on_properties_root(create_status_object("TRUE"));
}

void device_json_cfg_mode_password::set_false(void) {
  apply_on_properties_root(create_status_object("FALSE"));
}

void device_json_cfg_mode_password::set_waiting(void) {
  apply_on_properties_root(create_status_object("WAITING"));
}

void device_json_cfg_mode_password::set_unauthorized(void) {
  apply_on_properties_root(create_status_object("UNAUTHORIZED"));
}

void device_json_cfg_mode_password::set_not_supported(void) {
  apply_on_properties_root(create_status_object("NOT_SUPPORTED"));
}

bool device_json_cfg_mode_password::set_calcfg_result(
    TDS_DeviceCalCfgResult *result) {
  if (!result || result->Command != SUPLA_CALCFG_CMD_SET_CFG_MODE_PASSWORD) {
    return false;
  }

  if (result->DataSize == 0) {
    set_false();

    switch (result->Result) {
      case SUPLA_CALCFG_RESULT_TRUE:
      case SUPLA_CALCFG_RESULT_DONE:
        set_true();
        break;
      case SUPLA_CALCFG_RESULT_UNAUTHORIZED:
        set_unauthorized();
        break;
      case SUPLA_CALCFG_RESULT_NOT_SUPPORTED:
        set_not_supported();
        break;
    }

    return true;
  }

  return false;
}
