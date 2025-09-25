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

#include "device_json_pairing_result.h"

#include <string>

using std::string;

device_json_pairing_result::device_json_pairing_result(void) {}

device_json_pairing_result::~device_json_pairing_result(void) {}

std::time_t device_json_pairing_result::get_time(void) {
  return std::time(nullptr);
}

void device_json_pairing_result::add_time_object(cJSON* root, const char* key,
                                                 int inc_seconds) {
  char buffer[25] = {};

  std::time_t now = get_time();
  now += inc_seconds;
  std::tm utc_time;
  gmtime_r(&now, &utc_time);
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ",
                &utc_time);  // ISO 8601 UTC

  cJSON_AddStringToObject(root, key ? key : "time", buffer);
}

char* device_json_pairing_result::pairing_request_sent(void) {
  cJSON* json = cJSON_CreateObject();
  add_time_object(json, nullptr, 0);
  cJSON_AddStringToObject(json, "result", "REQUEST_SENT");

  char* json_str = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);

  return json_str;
}

char* device_json_pairing_result::calcfg_result_to_json(
    TDS_DeviceCalCfgResult* result) {
  if (!result || result->Command != SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING) {
    return nullptr;
  }

  string result_string;
  cJSON* json = cJSON_CreateObject();

  add_time_object(json, nullptr, 0);

  switch (result->Result) {
    case SUPLA_CALCFG_RESULT_NOT_SUPPORTED:
      result_string = "NOT_SUPPORTED";
      break;
    case SUPLA_CALCFG_RESULT_UNAUTHORIZED:
      result_string = "UNAUTHORIZED";
      break;
    case SUPLA_CALCFG_RESULT_TRUE:
      break;
    default:
      result_string = "CMD_RESULT_";
      result_string.append(std::to_string(result->Result));
      break;
  }

  if (result->Result == SUPLA_CALCFG_RESULT_TRUE &&
      result->DataSize >= sizeof(TCalCfg_SubdevicePairingResult) -
                              SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE &&
      result->DataSize <= sizeof(TCalCfg_SubdevicePairingResult)) {
    TCalCfg_SubdevicePairingResult* pairing_result =
        (TCalCfg_SubdevicePairingResult*)result->Data;

    int name_size = pairing_result->NameSize;
    if (name_size > SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE) {
      name_size = SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE;
    }

    if (name_size < 1) {
      name_size = 1;
    }

    string name(pairing_result->Name, name_size);

    cJSON_AddStringToObject(json, "name", name.c_str());

    add_time_object(json, "startedAt", pairing_result->ElapsedTimeSec * -1);
    add_time_object(
        json, "timeoutAt",
        pairing_result->MaximumDurationSec - pairing_result->ElapsedTimeSec);

    switch (pairing_result->PairingResult) {
      case SUPLA_CALCFG_PAIRINGRESULT_PROCEDURE_STARTED:
        result_string = "PROCEDURE_STARTED";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_ONGOING:
        result_string = "ONGOING";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_NO_NEW_DEVICE_FOUND:
        result_string = "NO_NEW_DEVICE_FOUND";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_SUCCESS:
        result_string = "SUCCESS";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_DEVICE_NOT_SUPPORTED:
        result_string = "DEVICE_NOT_SUPPORTED";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_RESOURCES_LIMIT_EXCEEDED:
        result_string = "RESOURCES_LIMIT_EXCEEDED";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_NOT_READY:
        result_string = "NOT_STARTED_NOT_READY";
        break;
      case SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_BUSY:
        result_string = "NOT_STARTED_BUSY";
        break;
      default:
        result_string = "RAIRING_RESULT_";
        result_string.append(std::to_string(pairing_result->PairingResult));
        break;
    }
  }

  cJSON_AddStringToObject(json, "result", result_string.c_str());
  result_string = "";

  char* json_str = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);

  return json_str;
}
