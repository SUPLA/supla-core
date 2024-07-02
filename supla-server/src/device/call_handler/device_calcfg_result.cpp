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

#include "device/call_handler/device_calcfg_result.h"

#include <memory>

#include "client.h"
#include "device.h"
#include "device/device_dao.h"
#include "json/cJSON.h"
#include "user.h"

using std::shared_ptr;
using std::string;

supla_ch_device_calcfg_result::supla_ch_device_calcfg_result(void)
    : supla_abstract_device_srpc_call_handler() {}

supla_ch_device_calcfg_result::~supla_ch_device_calcfg_result() {}

bool supla_ch_device_calcfg_result::can_handle_call(unsigned int call_id) {
  return call_id == SUPLA_DS_CALL_DEVICE_CALCFG_RESULT;
}

void supla_ch_device_calcfg_result::get_time_string(char* buffer,
                                                    size_t buffer_size,
                                                    int inc_seconds) {
  std::time_t now = std::time(nullptr);
  now += inc_seconds;
  std::tm utc_time;
  gmtime_r(&now, &utc_time);
  std::strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%SZ",
                &utc_time);  // ISO 8601 UTC
}

void supla_ch_device_calcfg_result::handle_call(
    shared_ptr<supla_device> device, supla_abstract_srpc_adapter* srpc_adapter,
    TsrpcReceivedData* rd, unsigned int call_id, unsigned char proto_version) {
  if (rd->data.ds_device_calcfg_result == nullptr) {
    return;
  }

  if (rd->data.ds_device_calcfg_result->Command ==
      SUPLA_CALCFG_CMD_ENTER_CFG_MODE) {
    device->entering_cfg_mode_in_progress = false;
    return;
  }

  int channel_id = device->get_channels()->get_channel_id(
      rd->data.ds_device_calcfg_result->ChannelNumber);

  if (channel_id == 0) {
    return;
  }

  if (rd->data.ds_device_calcfg_result->DataSize >=
          (sizeof(TCalCfg_ZWave_Node) - ZWAVE_NODE_NAME_MAXSIZE) &&
      rd->data.ds_device_calcfg_result->DataSize <=
          sizeof(TCalCfg_ZWave_Node)) {
    switch (rd->data.ds_device_calcfg_result->Command) {
      case SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE:
      case SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST:
        TCalCfg_ZWave_Node* node =
            (TCalCfg_ZWave_Node*)rd->data.ds_device_calcfg_result->Data;
        if (node->Flags & ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED) {
          node->ChannelID =
              device->get_channels()->get_channel_id(node->ChannelNumber);
          if (node->ChannelID == 0) {
            node->Flags ^= ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED;
          }
        } else {
          node->ChannelID = 0;
        }
        break;
    }
  }

  cJSON* json = cJSON_CreateObject();
  char buffer[25] = {};

  get_time_string(buffer, sizeof(buffer), 0);
  cJSON_AddStringToObject(json, "time", buffer);

  if (rd->data.ds_device_calcfg_result->Command ==
      SUPLA_CALCFG_CMD_START_SUBDEVICE_PAIRING) {
    string result;
    switch (rd->data.ds_device_calcfg_result->Result) {
      case SUPLA_CALCFG_RESULT_NOT_SUPPORTED:
        result = "NOT_SUPPORTED";
        break;
      case SUPLA_CALCFG_RESULT_UNAUTHORIZED:
        result = "UNAUTHORIZED";
        break;
      case SUPLA_CALCFG_RESULT_TRUE:
        break;
      default:
        result = "CMD_RESULT_";
        result.append(std::to_string(rd->data.ds_device_calcfg_result->Result));
        break;
    }

    if (rd->data.ds_device_calcfg_result->Result == SUPLA_CALCFG_RESULT_TRUE &&
        rd->data.ds_device_calcfg_result->DataSize >=
            sizeof(TCalCfg_SubdevicePairingResult) -
                SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE &&
        rd->data.ds_device_calcfg_result->DataSize <=
            sizeof(TCalCfg_SubdevicePairingResult)) {
      TCalCfg_SubdevicePairingResult* pairing_result =
          (TCalCfg_SubdevicePairingResult*)
              rd->data.ds_device_calcfg_result->Data;

      int name_size = pairing_result->NameSize;
      if (name_size > SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE) {
        name_size = SUPLA_CALCFG_SUBDEVICE_NAME_MAXSIZE;
      }

      if (name_size < 1) {
        name_size = 1;
      }

      pairing_result->Name[name_size - 1] = 0;
      cJSON_AddStringToObject(json, "name", pairing_result->Name);

      get_time_string(buffer, sizeof(buffer),
                      pairing_result->ElapsedTimeSec * -1);
      cJSON_AddStringToObject(json, "startedAt", buffer);

      get_time_string(
          buffer, sizeof(buffer),
          pairing_result->MaximumDurationSec - pairing_result->ElapsedTimeSec);

      cJSON_AddStringToObject(json, "timeoutAt", buffer);

      switch (pairing_result->PairingResult) {
        case SUPLA_CALCFG_PAIRINGRESULT_PROCEDURE_STARTED:
          result = "PROCEDURE_STARTED";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_ONGOING:
          result = "ONGOING";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_NO_NEW_DEVICE_FOUND:
          result = "NO_NEW_DEVICE_FOUND";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_SUCCESS:
          result = "SUCCESS";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_DEVICE_NOT_SUPPORTED:
          result = "NOT_SUPPORTED";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_RESOURCES_LIMIT_EXCEEDED:
          result = "RESOURCES_LIMIT_EXCEEDED";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_NOT_READY:
          result = "NOT_STARTED_NOT_READY";
          break;
        case SUPLA_CALCFG_PAIRINGRESULT_NOT_STARTED_BUSY:
          result = "NOT_STARTED_BUSY";
          break;
        default:
          result = "RAIRING_RESULT_";
          result.append(std::to_string(pairing_result->PairingResult));
          break;
      }
    }

    cJSON_AddStringToObject(json, "result", result.c_str());
    char* json_str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);

    supla_db_access_provider dba;
    supla_device_dao dao(&dba);
    dao.update_device_pairing_result(device->get_id(), json_str);

    free(json_str);
  }

  shared_ptr<supla_client> client = device->get_user()->get_clients()->get(
      rd->data.ds_device_calcfg_result->ReceiverID);
  if (client != nullptr) {
    client->on_device_calcfg_result(channel_id,
                                    rd->data.ds_device_calcfg_result);
  }
}
