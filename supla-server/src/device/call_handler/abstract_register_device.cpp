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

#include "device/call_handler/abstract_register_device.h"

#include <sys/syscall.h>
#include <unistd.h>

#include "log.h"

supla_ch_abstract_register_device::supla_ch_abstract_register_device(void) {}

supla_ch_abstract_register_device::~supla_ch_abstract_register_device() {}

char supla_ch_abstract_register_device::register_device(
    TDS_SuplaRegisterDevice_C *register_device_c,
    TDS_SuplaRegisterDevice_E *register_device_e, unsigned char proto_version) {
  int resultcode = SUPLA_RESULTCODE_TEMPORARILY_UNAVAILABLE;
  char result = 0;

  char *GUID = nullptr;
  char *AuthKey = nullptr;
  char *Name = nullptr;
  char *SoftVer = nullptr;
  unsigned char channel_count = 0;
  TDS_SuplaDeviceChannel_B *dev_channels_b = nullptr;
  TDS_SuplaDeviceChannel_C *dev_channels_c = nullptr;
  int LocationID = 0;
  int DeviceFlags = 0;
  short ManufacturerID = 0;
  short ProductID = 0;
  int flags = 0;

  if (register_device_c != nullptr) {
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
    flags = register_device_e->Flags;
  }

  set_flags(flags);

  if (!set_guid(GUID)) {
    resultcode = SUPLA_RESULTCODE_GUID_ERROR;

  } else if (register_device_e != nullptr &&
             !set_authkey(register_device_e->AuthKey)) {
    resultcode = SUPLA_RESULTCODE_AUTHKEY_ERROR;

  } else {
    supla_log(LOG_INFO,
              "Device registration started. ClientSD: %i "
              "Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X",
              get_client_sd(), proto_version, syscall(__NR_gettid),
              (unsigned char)GUID[0], (unsigned char)GUID[1],
              (unsigned char)GUID[2], (unsigned char)GUID[3]);

    if (db_connect()) {
      int UserID = 0;
      bool LocationEnabled = false;
      bool DeviceEnabled = true;
      int _LocationID = 0;
      int _OriginalLocationID = 0;
      bool new_device = false;
      bool channels_added = false;

      if (register_device_c != nullptr &&
          db->location_auth(LocationID, register_device_c->LocationPWD, &UserID,
                            &LocationEnabled) == false) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

      } else if (register_device_e != nullptr &&
                 false == authkey_auth(GUID, register_device_e->Email, AuthKey,
                                       &UserID, db)) {
        resultcode = SUPLA_RESULTCODE_BAD_CREDENTIALS;

        supla_log(LOG_INFO,
                  "(AUTHKEY_AUTH) Bad device credentials. ClientSD: %i "
                  "Protocol Version: %i "
                  "ThreadID: %i GUID: %02X%02X%02X%02X",
                  get_connection()->get_client_sd(),
                  get_connection()->get_protocol_version(),
                  syscall(__NR_gettid), (unsigned char)GUID[0],
                  (unsigned char)GUID[1], (unsigned char)GUID[2],
                  (unsigned char)GUID[3]);

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
            if (LocationID == 0 && register_device_e != nullptr) {
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
                                        get_connection()->get_client_ipv4(),
                                        SoftVer, proto_version, ManufacturerID,
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
              _supla_int_t DefaultFunc = 0;
              _supla_int_t ChannelFlags = 0;

              if (dev_channels_b != nullptr) {
                Number = dev_channels_b[a].Number;
                Type = dev_channels_b[a].Type;
                FuncList = dev_channels_b[a].FuncList;
                DefaultFunc = dev_channels_b[a].Default;
              } else {
                Number = dev_channels_c[a].Number;
                Type = dev_channels_c[a].Type;
                FuncList = dev_channels_c[a].FuncList;
                DefaultFunc = dev_channels_c[a].Default;
                ChannelFlags = dev_channels_c[a].Flags;
              }

              if (Type == 0) {
                break;
              }

              if (db->get_device_channel(DeviceID, Number, &ChannelType) == 0) {
                ChannelType = 0;
              }

              if (Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER &&
                  DefaultFunc == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
                // Issue #115
                DefaultFunc = SUPLA_CHANNELFNC_IC_ELECTRICITY_METER;
              }

              if (ChannelType == 0) {
                bool new_channel = false;

                int Param1 = 0;
                int Param2 = 0;
                supla_device_channel::getDefaults(Type, DefaultFunc, &Param1,
                                                  &Param2);

                int ChannelID = db->add_device_channel(
                    DeviceID, Number, Type, DefaultFunc, Param1, Param2,
                    supla_device_channel::funcListFilter(FuncList, Type),
                    ChannelFlags, UserID, &new_channel);

                if (ChannelID == 0) {
                  ChannelCount = -1;
                  break;
                } else if (new_channel) {
                  channels_added = true;
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
              if (AuthKey != nullptr) {
                _OriginalLocationID = 0;
              } else {
                if (LocationID == _LocationID) _OriginalLocationID = LocationID;
              }

              DeviceID =
                  db->update_device(DeviceID, _OriginalLocationID, AuthKey,
                                    Name, get_connection()->get_client_ipv4(),
                                    SoftVer, proto_version, flags);
            }

            if (DeviceID != 0) {
              db->commit();

              set_id(DeviceID);
              set_user(supla_user::find(UserID, true));

              load_config(UserID);

              channels->update_channels(dev_channels_b, dev_channels_c,
                                        channel_count);

              resultcode = SUPLA_RESULTCODE_TRUE;
              result = 1;
              supla_user::add_device(get_shared_ptr(), UserID);
              get_user()->get_clients()->update_device_channels(LocationID,
                                                                DeviceID);

              channels->on_device_registered(get_user(), DeviceID,
                                             dev_channels_b, dev_channels_c,
                                             channel_count);

              if (channels_added) {
                get_user()->on_channels_added(DeviceID,
                                              supla_caller(ctDevice, DeviceID));
              }

              get_user()->on_device_registered(
                  DeviceID, supla_caller(ctDevice, DeviceID));
            }
          }
        }
      }
    }
  }

  if (resultcode == SUPLA_RESULTCODE_TRUE) {
    supla_log(LOG_INFO,
              "Device registered. ID: %i, ClientSD: %i Protocol Version: %i "
              "ThreadID: %i GUID: %02X%02X%02X%02X",
              get_id(), get_connection()->get_client_sd(),
              get_connection()->get_protocol_version(), syscall(__NR_gettid),
              (unsigned char)GUID[0], (unsigned char)GUID[1],
              (unsigned char)GUID[2], (unsigned char)GUID[3]);
  } else {
    usleep(2000000);
  }

  TSD_SuplaRegisterDeviceResult srdr;
  srdr.result_code = resultcode;
  srdr.activity_timeout = get_connection()->get_activity_timeout();
  srdr.version_min = SUPLA_PROTO_VERSION_MIN;
  srdr.version = SUPLA_PROTO_VERSION;
  srpc_sd_async_registerdevice_result(
      get_connection()->get_srpc_adapter()->get_srpc(), &srdr);

  return result;
}
