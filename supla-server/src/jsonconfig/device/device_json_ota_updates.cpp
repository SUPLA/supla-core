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

#include "device_json_ota_updates.h"

#include <string.h>

#include "log.h"

#define FIELD_OTA_UPDATE 1

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_ota_updates::field_map =
    {{FIELD_OTA_UPDATE, "otaUpdate"}};

device_json_ota_updates::device_json_ota_updates(void)
    : device_json_async_result() {}

device_json_ota_updates::device_json_ota_updates(supla_json_config *root)
    : device_json_async_result(root) {}

device_json_ota_updates::~device_json_ota_updates(void) {}

string device_json_ota_updates::get_status_root_key(void) {
  return field_map.at(FIELD_OTA_UPDATE);
}

const map<unsigned _supla_int16_t, string> &
device_json_ota_updates::get_field_map(void) {
  return field_map;
}

void device_json_ota_updates::set_checking(void) {
  apply_on_properties_root(create_status_object("checking"));
}

void device_json_ota_updates::set_not_available(void) {
  apply_on_properties_root(create_status_object("notAvailable"));
}

void device_json_ota_updates::set_available(string soft_ver,
                                            string changelog_url) {
  cJSON *ota = create_status_object("available");
  if (ota) {
    cJSON_AddStringToObject(ota, "version", soft_ver.c_str());
    cJSON_AddStringToObject(ota, "url", changelog_url.c_str());
    apply_on_properties_root(ota);
  }
}

void device_json_ota_updates::set_error(void) {
  apply_on_properties_root(create_status_object("error"));
}

bool device_json_ota_updates::set_calcfg_result(
    TDS_DeviceCalCfgResult *result) {
  if (!result || result->Command != SUPLA_CALCFG_CMD_CHECK_FIRMWARE_UPDATE) {
    return false;
  }

  set_error();

  if (sizeof(TCalCfg_FirmwareCheckResult) == result->DataSize) {
    TCalCfg_FirmwareCheckResult *fw_check_result =
        (TCalCfg_FirmwareCheckResult *)result->Data;

    if (fw_check_result->Result ==
        SUPLA_FIRMWARE_CHECK_RESULT_UPDATE_NOT_AVAILABLE) {
      set_not_available();
    } else if (fw_check_result->Result ==
               SUPLA_FIRMWARE_CHECK_RESULT_UPDATE_AVAILABLE) {
      string soft_ver(fw_check_result->SoftVer,
                      sizeof(fw_check_result->SoftVer) - 1);
      string changelog_url(fw_check_result->ChangelogUrl,
                           sizeof(fw_check_result->ChangelogUrl) - 1);

      set_available(soft_ver, changelog_url);
    }

    return true;
  }

  return false;
}
