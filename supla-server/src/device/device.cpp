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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include "database.h"
#include "device.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

supla_device::supla_device(serverconnection *svrconn) : cdbase(svrconn) {
  this->channels = new supla_device_channels();
}

supla_device::~supla_device() {
  if (getUser()) {  // 1st line!
    std::list<int> ids = channels->get_channel_ids();
    for (std::list<int>::iterator it = ids.begin(); it != ids.end(); it++) {
      getUser()->on_channel_value_changed(EST_DEVICE, getID(), *it);
    }
  }

  delete channels;
}

// static
bool supla_device::funclist_contains_function(int funcList, int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGATEWAYLOCK) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGATE) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEGARAGEDOOR) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEDOORLOCK) > 0;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      return (funcList & SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER) > 0;
    case SUPLA_CHANNELFNC_POWERSWITCH:
      return (funcList & SUPLA_BIT_FUNC_POWERSWITCH) > 0;
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      return (funcList & SUPLA_BIT_FUNC_LIGHTSWITCH) > 0;
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return (funcList & SUPLA_BIT_FUNC_STAIRCASETIMER) > 0;
    case SUPLA_CHANNELFNC_THERMOMETER:
      return (funcList & SUPLA_BIT_FUNC_THERMOMETER) > 0;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      return (funcList & SUPLA_BIT_FUNC_HUMIDITYANDTEMPERATURE) > 0;
    case SUPLA_CHANNELFNC_HUMIDITY:
      return (funcList & SUPLA_BIT_FUNC_HUMIDITY) > 0;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      return (funcList & SUPLA_BIT_FUNC_WINDSENSOR) > 0;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      return (funcList & SUPLA_BIT_FUNC_PRESSURESENSOR) > 0;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      return (funcList & SUPLA_BIT_FUNC_RAINSENSOR) > 0;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      return (funcList & SUPLA_BIT_FUNC_WEIGHTSENSOR) > 0;
  }

  return false;
}

bool supla_device::db_authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                                   const char Email[SUPLA_EMAIL_MAXSIZE],
                                   const char AuthKey[SUPLA_AUTHKEY_SIZE],
                                   int *UserID, database *db) {
  return db->device_authkey_auth(GUID, Email, AuthKey, UserID);
}

char supla_device::register_device(TDS_SuplaRegisterDevice_C *register_device_c,
                                   TDS_SuplaRegisterDevice_E *register_device_e,
                                   unsigned char proto_version) {
  int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
  char result = 0;

  char *GUID = NULL;
  char *AuthKey = NULL;
  char *Name = NULL;
  char *SoftVer = NULL;
  unsigned char channel_count = 0;
  TDS_SuplaDeviceChannel_B *dev_channels_b = NULL;
  TDS_SuplaDeviceChannel_C *dev_channels_c = NULL;
  int LocationID = 0;
  int DeviceFlags = 0;
  short ManufacturerID = 0;
  short ProductID = 0;

  if (register_device_c != NULL) {
    GUID = register_device_c->GUID;
    Name = register_device_c->Name;
    SoftVer = register_device_c->SoftVer;
    channel_count = register_device_c->channel_count;
    dev_channels_b = register_device_c->channels;
    LocationID = register_device_c->LocationID;
  } else {
    GUID = register_device_e->GUID;
    AuthKey = register_device_e->AuthKey;
    Name = register_device_e->Name;
    SoftVer = register_device_e->SoftVer;
    DeviceFlags = register_device_e->Flags;
    channel_count = register_device_e->channel_count;
    dev_channels_c = register_device_e->channels;
    ManufacturerID = register_device_e->ManufacturerID;
    ProductID = register_device_e->ProductID;
  }

  if (!setGUID(GUID)) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_device_e != NULL &&
             !setAuthKey(register_device_e->AuthKey)) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else {
    database *db = new database();

    if (db->connect() == true) {
      int UserID = 0;
      bool LocationEnabled = false;
      bool DeviceEnabled = true;
      int _LocationID = 0;
      int _OriginalLocationID = 0;
      bool new_device = false;

      if (register_device_c != NULL &&
          db->location_auth(LocationID, register_device_c->LocationPWD, &UserID,
                            &LocationEnabled) == false) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (register_device_e != NULL &&
                 false == authkey_auth(GUID, register_device_e->Email, AuthKey,
                                       &UserID, db)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (UserID == 0) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else {
        if (strnlen(Name, SUPLA_DEVICE_NAME_MAXSIZE - 1) < 1) {
          snprintf(Name, SUPLA_DEVICE_NAME_MAXSIZE, "unknown");
        }

        db->start_transaction();

        int DeviceID = db->get_device(db->get_device_id(UserID, GUID),
                                      &DeviceEnabled, &_OriginalLocationID,
                                      &_LocationID, &LocationEnabled);

        if (LocationID == 0) LocationID = _LocationID;

        if (DeviceID == 0) {
          if (false == db->get_device_reg_enabled(UserID)) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_REGISTRATION_DISABLED;

          } else if (db->get_device_limit_left(UserID) <= 0) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_DEVICE_LIMITEXCEEDED;

          } else {
            if (LocationID == 0 && register_device_e != NULL) {
              if ((LocationID = db->get_location_id(UserID, true)) != 0) {
                LocationEnabled = true;

              } else if ((LocationID = db->get_location_id(UserID, false)) !=
                         0) {
                LocationEnabled = false;

              } else {
                db->rollback();
                resultcode = SUPLA_RESULTCODE_NO_LOCATION_AVAILABLE;
              }
            }

            if (LocationID != 0) {
              new_device = true;

              _LocationID = LocationID;

              DeviceID = db->add_device(LocationID, GUID, AuthKey, Name,
                                        getSvrConn()->getClientIpv4(), SoftVer,
                                        proto_version, ManufacturerID,
                                        ProductID, DeviceFlags, UserID);
            }
          }
        }

        if (DeviceID != 0) {
          if (!DeviceEnabled) {
            DeviceID = 0;
            db->rollback();
            resultcode = SUPLA_RESULTCODE_DEVICE_DISABLED;

          } else if (!LocationEnabled) {
            DeviceID = 0;
            db->rollback();
            resultcode = SUPLA_RESULTCODE_LOCATION_DISABLED;

          } else if (LocationID == 0 || (LocationID != _LocationID &&
                                         LocationID != _OriginalLocationID)) {
            DeviceID = 0;
            db->rollback();
            resultcode = SUPLA_RESULTCODE_LOCATION_CONFLICT;
          }
        }

        if (DeviceID != 0) {
          int ChannelCount = 0;
          int ChannelType = 0;

          for (int a = 0; a < SUPLA_CHANNELMAXCOUNT; a++)
            if (a >= channel_count) {
              break;
            } else {
              ChannelCount++;

              unsigned char Number = 0;
              _supla_int_t Type = 0;
              _supla_int_t FuncList = 0;
              _supla_int_t Default = 0;
              _supla_int_t ChannelFlags = 0;

              if (dev_channels_b != NULL) {
                Number = dev_channels_b[a].Number;
                Type = dev_channels_b[a].Type;
                FuncList = dev_channels_b[a].FuncList;
                Default = dev_channels_b[a].Default;
              } else {
                Number = dev_channels_c[a].Number;
                Type = dev_channels_c[a].Type;
                FuncList = dev_channels_c[a].FuncList;
                Default = dev_channels_c[a].Default;
                ChannelFlags = dev_channels_c[a].Flags;
              }

              if (Type == 0) {
                break;
              }

              if (db->get_device_channel(DeviceID, Number, &ChannelType) == 0) {
                ChannelType = 0;
              }
#ifndef SERVER_VERSION_23
              if (Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
                  Default == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
                // Issue #115
                Default = SUPLA_CHANNELFNC_IC_ELECTRICITY_METER;
              }
#endif /*SERVER_VERSION_23*/

              if (ChannelType == 0) {
                bool new_channel = false;
                int ChannelID = db->add_device_channel(
                    DeviceID, Number, Type, Default, FuncList, ChannelFlags,
                    UserID, &new_channel);

                if (ChannelID == 0) {
                  ChannelCount = -1;
                  break;
                } else if (new_channel) {
                  db->on_channeladded(DeviceID, ChannelID);
                }

              } else if (ChannelType != Type) {
                ChannelCount = -1;
                break;
              }
            }

          if (ChannelCount == -1 ||
              db->get_device_channel_count(DeviceID) != ChannelCount) {
            db->rollback();
            resultcode = SUPLA_RESULTCODE_CHANNEL_CONFLICT;

          } else {
            if (new_device) {
              db->on_newdevice(DeviceID);
            } else {
              if (AuthKey != NULL) {
                _OriginalLocationID = 0;
              } else {
                if (LocationID == _LocationID) _OriginalLocationID = LocationID;
              }

              DeviceID = db->update_device(
                  DeviceID, _OriginalLocationID, AuthKey, Name,
                  getSvrConn()->getClientIpv4(), SoftVer, proto_version);
            }

            if (DeviceID != 0) {
              db->commit();

              setID(DeviceID);

              load_config(UserID);

              channels->set_channels_value(dev_channels_b, dev_channels_c,
                                           channel_count);

              resultcode = SUPLA_RESULTCODE_TRUE;
              result = 1;
              setUser(supla_user::add_device(this, UserID));
              getUser()->update_client_device_channels(LocationID, getID());

              channels->on_device_registered(getUser(), DeviceID,
                                             dev_channels_b, dev_channels_c,
                                             channel_count);

              if (new_device) {
                getUser()->on_device_added(DeviceID, EST_DEVICE);
              }
            }
          }
        }
      }
    }

    delete db;
  }

  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    supla_log(LOG_INFO,
              "Device registered. ID: %i, ClientSD: %i Protocol Version: %i "
              "ThreadID: %i",
              getID(), getSvrConn()->getClientSD(),
              getSvrConn()->getProtocolVersion(), syscall(__NR_gettid));
  } else {
    usleep(2000000);
  }

  TSD_SuplaRegisterDeviceResult srdr;
  srdr.result_code = resultcode;
  srdr.activity_timeout = getSvrConn()->GetActivityTimeout();
  srdr.version_min = SUPLA_PROTO_VERSION_MIN;
  srdr.version = SUPLA_PROTO_VERSION;
  srpc_sd_async_registerdevice_result(getSvrConn()->srpc(), &srdr);

  return result;
}

void supla_device::load_config(int UserID) { channels->load(UserID, getID()); }

void supla_device::on_device_channel_value_changed(
    TDS_SuplaDeviceChannelValue *value, TDS_SuplaDeviceChannelValue_B *value_b,
    TDS_SuplaDeviceChannelValue_C *value_c) {
  if (value == NULL && value_b == NULL && value_c == NULL) {
    return;
  }

  unsigned char ChannelNumber = 0;
  char *value_value = NULL;
  bool offline = false;

  if (value_c) {
    ChannelNumber = value_c->ChannelNumber;
    value_value = value_c->value;
    offline = value_c->Offline > 0;
  } else if (value_b) {
    ChannelNumber = value_b->ChannelNumber;
    value_value = value_b->value;
    offline = value_b->Offline > 0;
  } else if (value) {
    ChannelNumber = value->ChannelNumber;
    value_value = value->value;
  }

  int ChannelId = channels->get_channel_id(ChannelNumber);

  if (ChannelId != 0) {
    bool converted2extended;
    channels->set_channel_value(ChannelId, value_value, &converted2extended,
                                value_c ? &value_c->ValidityTimeSec : NULL);
    if (value_b || value_c) {
      channels->set_channel_offline(ChannelId, offline);
    }
    getUser()->on_channel_value_changed(EST_DEVICE, getID(), ChannelId);

    if (converted2extended) {
      getUser()->on_channel_value_changed(EST_DEVICE, getID(), ChannelId, true);
    }
  }
}

void supla_device::on_device_channel_extendedvalue_changed(
    TDS_SuplaDeviceChannelExtendedValue *ev) {
  int ChannelId = channels->get_channel_id(ev->ChannelNumber);

  if (ChannelId != 0) {
    channels->set_channel_extendedvalue(ChannelId, &ev->value);
    getUser()->on_channel_value_changed(EST_DEVICE, getID(), ChannelId, true);
  }
}

void supla_device::on_channel_set_value_result(
    TDS_SuplaChannelNewValueResult *result) {
  if (result == NULL || result->SenderID == 0) {
    return;
  }

  int ChannelID = channels->get_channel_id(result->ChannelNumber);

  if (ChannelID == 0) {
    return;
  }

  int ChannelType = channels->get_channel_type(ChannelID);

  if (result->Success == 1 || ChannelType == SUPLA_CHANNELTYPE_BRIDGE) {
    TSC_SuplaEvent event;
    memset(&event, 0, sizeof(TSC_SuplaEvent));
    event.ChannelID = ChannelID;
    event.SenderID = result->SenderID;
    event.DurationMS = channels->get_channel_value_duration(ChannelID);
    getUser()->getClientName(result->SenderID, event.SenderName,
                             SUPLA_SENDER_NAME_MAXSIZE);
    event.SenderNameSize =
        strnlen(event.SenderName, SUPLA_SENDER_NAME_MAXSIZE - 1) + 1;

    if (result->Success == 0 && ChannelType == SUPLA_CHANNELTYPE_BRIDGE) {
      event.Event = SUPLA_EVENT_SET_BRIDGE_VALUE_FAILED;
    } else {
      switch (channels->get_channel_func(ChannelID)) {
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGATE;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEDOORLOCK;
          break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
          event.Event = SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER;
          break;
        case SUPLA_CHANNELFNC_POWERSWITCH:
          event.Event = SUPLA_EVENT_POWERONOFF;
          break;
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
          event.Event = SUPLA_EVENT_LIGHTONOFF;
          break;
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
          event.Event = SUPLA_EVENT_STAIRCASETIMERONOFF;
          break;
        case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
          event.Event = SUPLA_EVENT_VALVEOPENCLOSE;
          break;
      }
    }

    getUser()->call_event(&event);
  }
}

bool supla_device::get_channel_value(int ChannelID,
                                     char value[SUPLA_CHANNELVALUE_SIZE],
                                     char *online,
                                     unsigned _supla_int_t *validity_time_sec) {
  return channels->get_channel_value(ChannelID, value, online,
                                     validity_time_sec);
}

bool supla_device::get_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *value) {
  return channels->get_channel_extendedvalue(ChannelID, value);
}

void supla_device::set_device_channel_value(
    int SenderID, int ChannelID, int GroupID, unsigned char EOL,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  channels->set_device_channel_value(getSvrConn()->srpc(), SenderID, ChannelID,
                                     GroupID, EOL, value);
}

bool supla_device::set_device_channel_char_value(int SenderID, int ChannelID,
                                                 int GroupID, unsigned char EOL,
                                                 const char value) {
  return channels->set_device_channel_char_value(
      getSvrConn()->srpc(), SenderID, ChannelID, GroupID, EOL, value);
}

bool supla_device::set_device_channel_rgbw_value(int SenderID, int ChannelID,
                                                 int GroupID, unsigned char EOL,
                                                 int color,
                                                 char color_brightness,
                                                 char brightness, char on_off) {
  return channels->set_device_channel_rgbw_value(
      getSvrConn()->srpc(), SenderID, ChannelID, GroupID, EOL, color,
      color_brightness, brightness, on_off);
}

bool supla_device::channel_exists(int ChannelID) {
  return channels->channel_exists(ChannelID);
}

std::list<int> supla_device::master_channel(int ChannelID) {
  return channels->master_channel(ChannelID);
}

std::list<int> supla_device::related_channel(int ChannelID) {
  return channels->related_channel(ChannelID);
}

bool supla_device::get_channel_double_value(int ChannelID, double *Value) {
  return channels->get_channel_double_value(ChannelID, Value);
}

bool supla_device::get_channel_temperature_value(int ChannelID, double *Value) {
  return channels->get_channel_temperature_value(ChannelID, Value);
}

bool supla_device::get_channel_humidity_value(int ChannelID, double *Value) {
  return channels->get_channel_humidity_value(ChannelID, Value);
}

void supla_device::get_temp_and_humidity(void *tarr) {
  channels->get_temp_and_humidity(tarr);
}

void supla_device::get_electricity_measurements(void *emarr) {
  channels->get_electricity_measurements(emarr);
}

supla_channel_electricity_measurement *
supla_device::get_electricity_measurement(int ChannelID) {
  return channels->get_electricity_measurement(ChannelID);
}

void supla_device::get_ic_measurements(void *icarr) {
  channels->get_ic_measurements(icarr);
}

supla_channel_ic_measurement *supla_device::get_ic_measurement(int ChannelID) {
  return channels->get_ic_measurement(ChannelID);
}

void supla_device::get_thermostat_measurements(void *tharr) {
  channels->get_thermostat_measurements(tharr);
}

bool supla_device::get_channel_char_value(int ChannelID, char *Value) {
  return channels->get_channel_char_value(ChannelID, Value);
}

bool supla_device::get_channel_rgbw_value(int ChannelID, int *color,
                                          char *color_brightness,
                                          char *brightness, char *on_off) {
  return channels->get_channel_rgbw_value(ChannelID, color, color_brightness,
                                          brightness, on_off);
}

bool supla_device::get_channel_valve_value(int ChannelID, TValve_Value *Value) {
  return channels->get_channel_valve_value(ChannelID, Value);
}

void supla_device::get_firmware_update_url(TDS_FirmwareUpdateParams *params) {
  TSD_FirmwareUpdate_UrlResult result;
  memset(&result, 0, sizeof(TSD_FirmwareUpdate_UrlResult));

  database *db = new database();

  if (db->connect() == true)
    db->get_device_firmware_update_url(getID(), params, &result);

  delete db;

  srpc_sd_async_get_firmware_update_url_result(getSvrConn()->srpc(), &result);
}

bool supla_device::calcfg_request(int SenderID, int ChannelID,
                                  bool SuperUserAuthorized,
                                  TCS_DeviceCalCfgRequest_B *request) {
  return channels->calcfg_request(getSvrConn()->srpc(), SenderID, ChannelID,
                                  SuperUserAuthorized, request);
}

void supla_device::on_calcfg_result(TDS_DeviceCalCfgResult *result) {
  int ChannelID = channels->get_channel_id(result->ChannelNumber);
  if (ChannelID != 0) {
    if (result->DataSize >=
            (sizeof(TCalCfg_ZWave_Node) - ZWAVE_NODE_NAME_MAXSIZE) &&
        result->DataSize <= sizeof(TCalCfg_ZWave_Node)) {
      switch (result->Command) {
        case SUPLA_CALCFG_CMD_ZWAVE_ADD_NODE:
        case SUPLA_CALCFG_CMD_ZWAVE_GET_NODE_LIST:
          TCalCfg_ZWave_Node *node = (TCalCfg_ZWave_Node *)result->Data;
          if (node->Flags & ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED) {
            node->ChannelID = channels->get_channel_id(node->ChannelNumber);
            if (node->ChannelID == 0) {
              node->Flags ^= ZWAVE_NODE_FLAG_CHANNEL_ASSIGNED;
            }
          } else {
            node->ChannelID = 0;
          }
          break;
      }
    }

    getUser()->on_device_calcfg_result(ChannelID, result);
  }
}

void supla_device::on_channel_state_result(TDSC_ChannelState *state) {
  int ChannelID;
  if ((ChannelID = channels->get_channel_id(state->ChannelNumber)) != 0) {
    getUser()->on_device_channel_state_result(ChannelID, state);
  }
}

bool supla_device::get_channel_state(int SenderID,
                                     TCSD_ChannelStateRequest *request) {
  return channels->get_channel_state(getSvrConn()->srpc(), SenderID, request);
}

bool supla_device::get_channel_complex_value(channel_complex_value *value,
                                             int ChannelID) {
  return channels->get_channel_complex_value(value, ChannelID);
}

void supla_device::set_channel_function(int ChannelId, int Func) {
  channels->set_channel_function(ChannelId, Func);
}

void supla_device::get_channel_functions_request(void *srpc) {
  channels->get_functions_request(srpc);
}
