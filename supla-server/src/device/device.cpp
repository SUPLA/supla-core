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

#include "database.h"
#include "device.h"
#include "http/httprequestqueue.h"
#include "lck.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user.h"

supla_device::supla_device(serverconnection *svrconn) : cdcommon(svrconn) {
  this->channels = new supla_device_channels();
}

supla_device::~supla_device() {
  if (getUser()) {  // 1st line!
    getUser()->remove_device(this);
    getUser()->on_channel_value_changed(EST_DEVICE, getID());
  }

  delete channels;
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
      int _UserID = 0;
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
                 false ==
                     db->device_authkey_auth(GUID, register_device_e->Email,
                                             AuthKey, &UserID)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (UserID == 0) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else {
        if (strnlen(Name, SUPLA_DEVICE_NAME_MAXSIZE - 1) < 1) {
          snprintf(Name, SUPLA_DEVICE_NAME_MAXSIZE, "unknown");
        }

        db->start_transaction();

        int DeviceID = db->get_device(db->get_device_id(GUID), &DeviceEnabled,
                                      &_OriginalLocationID, &_LocationID,
                                      &LocationEnabled, &_UserID);

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

              _UserID = UserID;
              _LocationID = LocationID;

              DeviceID = db->add_device(LocationID, GUID, AuthKey, Name,
                                        getSvrConn()->getClientIpv4(), SoftVer,
                                        proto_version, ManufacturerID,
                                        ProductID, DeviceFlags, UserID);
            }
          }
        }

        if (DeviceID != 0) {
          if (UserID != _UserID) {
            DeviceID = 0;
            db->rollback();
            resultcode = SUPLA_RESULTCODE_USER_CONFLICT;

          } else if (!DeviceEnabled) {
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

              if (db->get_device_channel(DeviceID, Number, &ChannelType) == 0) {
                ChannelType = 0;
              }

              if (ChannelType == 0) {
                bool new_channel = false;
                int ChannelID = db->add_device_channel(
                    DeviceID, Number, Type, Default ? Default : 0, FuncList,
                    ChannelFlags, UserID, &new_channel);

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

              load_config();

              channels->set_channels_value(dev_channels_b, dev_channels_c,
                                           channel_count);

              resultcode = SUPLA_RESULTCODE_TRUE;
              result = 1;
              setUser(supla_user::add_device(this, UserID));
              getUser()->update_client_device_channels(LocationID, getID());
            }
          }
        }
      }
    }

    delete db;
  }

  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    supla_log(LOG_INFO,
              "Device registered. ID: %i, ClientSD: %i Protocol Version: %i",
              getID(), getSvrConn()->getClientSD(),
              getSvrConn()->getProtocolVersion());
  } else {
    usleep(2000000);
  }

  TSD_SuplaRegisterDeviceResult srdr;
  srdr.result_code = resultcode;
  srdr.activity_timeout = getSvrConn()->GetActivityTimeout();
  srdr.version_min = SUPLA_PROTO_VERSION;
  srdr.version = SUPLA_PROTO_VERSION;
  srpc_sd_async_registerdevice_result(getSvrConn()->srpc(), &srdr);

  return result;
}

void supla_device::load_config(void) { channels->load(getID()); }

void supla_device::on_device_channel_value_changed(
    TDS_SuplaDeviceChannelValue *value) {
  int ChannelId = channels->get_channel_id(value->ChannelNumber);

  if (ChannelId != 0) {
    bool converted2extended;
    channels->set_channel_value(ChannelId, value->value, &converted2extended);
    getUser()->on_channel_value_changed(EST_DEVICE, getID(),
                                        ChannelId);

    if (converted2extended) {
      getUser()->on_channel_value_changed(EST_DEVICE, getID(),
                                          ChannelId, true);
    }
  }
}

void supla_device::on_device_channel_extendedvalue_changed(
    TDS_SuplaDeviceChannelExtendedValue *ev) {
  int ChannelId = channels->get_channel_id(ev->ChannelNumber);

  if (ChannelId != 0) {
    channels->set_channel_extendedvalue(ChannelId, &ev->value);
    getUser()->on_channel_value_changed(EST_DEVICE, getID(),
                                        ChannelId, true);
  }
}

void supla_device::on_channel_set_value_result(
    TDS_SuplaChannelNewValueResult *result) {
  int ChannelID;

  if (result->Success == 1 && result->SenderID != 0 &&
      (ChannelID = channels->get_channel_id(result->ChannelNumber)) != 0) {
    TSC_SuplaEvent event;
    memset(&event, 0, sizeof(TSC_SuplaEvent));
    event.ChannelID = ChannelID;
    event.SenderID = result->SenderID;
    event.DurationMS = channels->get_channel_value_duration(ChannelID);
    getUser()->getClientName(result->SenderID, event.SenderName,
                             SUPLA_SENDER_NAME_MAXSIZE);
    event.SenderNameSize =
        strnlen(event.SenderName, SUPLA_SENDER_NAME_MAXSIZE - 1) + 1;

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
    }

    getUser()->call_event(&event);
  }
}

bool supla_device::get_channel_value(int ChannelID,
                                     char value[SUPLA_CHANNELVALUE_SIZE]) {
  return channels->get_channel_value(ChannelID, value);
}

bool supla_device::get_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *value) {
  return channels->get_channel_extendedvalue(ChannelID, value);
}

void supla_device::set_device_channel_value(
    int SenderID, int ChannelID, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  channels->set_device_channel_value(getSvrConn()->srpc(), SenderID, ChannelID,
                                     value);
}

bool supla_device::set_device_channel_char_value(int SenderID, int ChannelID,
                                                 const char value) {
  return channels->set_device_channel_char_value(getSvrConn()->srpc(), SenderID,
                                                 ChannelID, value);
}

bool supla_device::set_device_channel_rgbw_value(int SenderID, int ChannelID,
                                                 int color,
                                                 char color_brightness,
                                                 char brightness) {
  return channels->set_device_channel_rgbw_value(getSvrConn()->srpc(), SenderID,
                                                 ChannelID, color,
                                                 color_brightness, brightness);
}

bool supla_device::channel_exists(int ChannelID) {
  return channels->channel_exists(ChannelID);
}

std::list<int> supla_device::master_channel(int ChannelID) {
  return channels->master_channel(ChannelID);
}

std::list<int> supla_device::slave_channel(int ChannelID) {
  return channels->slave_channel(ChannelID);
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

void supla_device::get_electricity_measurement(void *emarr) {
  channels->get_electricity_measurement(emarr);
}

bool supla_device::get_channel_char_value(int ChannelID, char *Value) {
  return channels->get_channel_char_value(ChannelID, Value);
}

bool supla_device::get_channel_rgbw_value(int ChannelID, int *color,
                                          char *color_brightness,
                                          char *brightness) {
  return channels->get_channel_rgbw_value(ChannelID, color, color_brightness,
                                          brightness);
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

bool supla_device::calcfg_request(int SenderID, bool SuperUserAuthorized,
                                  TCS_DeviceCalCfgRequest *request) {
  return channels->calcfg_request(getSvrConn()->srpc(), SenderID,
                                  SuperUserAuthorized, request);
}

void supla_device::on_calcfg_result(TDS_DeviceCalCfgResult *result) {
  int ChannelID;
  if ((ChannelID = channels->get_channel_id(result->ChannelNumber)) != 0) {
    getUser()->on_device_calcfg_result(ChannelID, result);
  }
}

bool supla_device::get_channel_complex_value(channel_complex_value *value,
                                             int ChannelID) {
  return channels->get_channel_complex_value(value, ChannelID);
}
