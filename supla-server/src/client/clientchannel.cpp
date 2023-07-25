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

#include "clientchannel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "client.h"
#include "db/database.h"
#include "device/devicechannel.h"
#include "log.h"
#include "proto.h"
#include "safearray.h"
#include "srpc/srpc.h"
#include "user.h"

using std::shared_ptr;

supla_client_channel::supla_client_channel(
    supla_client_channels *Container, int Id, unsigned char channel_number,
    int DeviceId, int LocationID, int Type, int Func, int Param1, int Param2,
    int Param3, int Param4, char *TextParam1, char *TextParam2,
    char *TextParam3, const char *Caption, int AltIcon, int UserIcon,
    short ManufacturerID, short ProductID, unsigned char ProtocolVersion,
    int Flags, int EmSubcFlags, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec, const char *user_config,
    const char *em_subc_user_config)
    : supla_client_objcontainer_item(Container, Id, Caption),
      supla_abstract_common_channel_properties() {
  this->channel_number = channel_number;
  this->DeviceId = DeviceId;
  this->LocationId = LocationID;
  this->Type = Type;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->Param3 = Param3;
  this->Param4 = Param4;
  this->TextParam1 = TextParam1 ? strndup(TextParam1, 255) : nullptr;
  this->TextParam2 = TextParam2 ? strndup(TextParam2, 255) : nullptr;
  this->TextParam3 = TextParam3 ? strndup(TextParam3, 255) : nullptr;
  this->AltIcon = AltIcon;
  this->UserIcon = UserIcon;
  this->ManufacturerID = ManufacturerID;
  this->ProductID = ProductID;
  this->ProtocolVersion = ProtocolVersion;
  this->Flags = Flags;

  this->Flags |= EmSubcFlags & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED;
  this->Flags |= EmSubcFlags & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED;
  this->Flags |= EmSubcFlags & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED;

  json_config = new channel_json_config();
  json_config->set_user_config(user_config);

  if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    electricity_meter_config *config = new electricity_meter_config();

    config->set_user_config(user_config);
    this->Flags |= config->get_channel_user_flags();

    config->set_user_config(em_subc_user_config);
    this->Flags |= config->get_channel_user_flags();

    delete config;
  }

  this->Flags ^= this->Flags & SUPLA_CHANNEL_FLAG_HAS_PARENT;

  setValueValidityTimeSec(validity_time_sec);
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);
}

supla_client_channel::~supla_client_channel(void) {
  if (this->TextParam1) {
    free(this->TextParam1);
    this->TextParam1 = nullptr;
  }

  if (this->TextParam2) {
    free(this->TextParam2);
    this->TextParam2 = nullptr;
  }

  if (this->TextParam3) {
    free(this->TextParam3);
    this->TextParam3 = nullptr;
  }

  delete json_config;
}

unsigned char supla_client_channel::get_channel_number(void) {
  return channel_number;
}

int supla_client_channel::get_device_id() { return DeviceId; }

int supla_client_channel::get_id() {
  return supla_client_objcontainer_item::get_id();
}

int supla_client_channel::get_extra_id() { return DeviceId; }

int supla_client_channel::get_type() { return Type; }

int supla_client_channel::get_func() { return Func; }

void supla_client_channel::set_func(int Func) {
  if (Func != this->Func) {
    this->Func = Func;
    mark_for_remote_update(OI_REMOTEUPDATE_DATA1);
  }
}

int supla_client_channel::get_param1() { return Param1; }

int supla_client_channel::get_param2() { return Param2; }

int supla_client_channel::get_param3() { return Param3; }

int supla_client_channel::get_param4() { return Param4; }

channel_json_config *supla_client_channel::get_json_config() {
  return new channel_json_config(json_config, true);
}

void supla_client_channel::setCaption(const char *Caption) {
  if ((Caption == nullptr && getCaption() != nullptr) ||
      (Caption != nullptr && getCaption() == nullptr) ||
      strncmp(Caption, getCaption(), SUPLA_CHANNEL_CAPTION_MAXSIZE) != 0) {
    supla_client_objcontainer_item::setCaption(Caption);
    mark_for_remote_update(OI_REMOTEUPDATE_DATA1);
  }
}

short supla_client_channel::get_manufacturer_id() { return ManufacturerID; }

short supla_client_channel::get_product_id() { return ProductID; }

int supla_client_channel::get_flags() {
  auto relations = get_channel_relations(relation_with_parent_channel);
  return Flags | (relations.size() ? SUPLA_CHANNEL_FLAG_HAS_PARENT : 0);
}

void supla_client_channel::setValueValidityTimeSec(
    unsigned _supla_int_t validity_time_sec) {
  resetValueValidityTime();

  if (validity_time_sec > 0) {
    gettimeofday(&value_valid_to, nullptr);
    value_valid_to.tv_sec += validity_time_sec;
  }
}

bool supla_client_channel::isValueValidityTimeSet() {
  return value_valid_to.tv_sec || value_valid_to.tv_usec;
}

unsigned _supla_int64_t supla_client_channel::getValueValidityTimeUSec(void) {
  if (isValueValidityTimeSet()) {
    struct timeval now;
    gettimeofday(&now, nullptr);

    _supla_int64_t result =
        (value_valid_to.tv_sec * 1000000 + value_valid_to.tv_usec) -
        (now.tv_sec * 1000000 + now.tv_usec);
    if (result > 0) {
      return result;
    }
  }

  return 0;
}

void supla_client_channel::resetValueValidityTime(void) {
  value_valid_to.tv_sec = 0;
  value_valid_to.tv_usec = 0;
}

bool supla_client_channel::remote_update_is_possible(void) {
  int protocol_version = dynamic_cast<supla_client_channels *>(getContainer())
                             ->getClient()
                             ->get_protocol_version();
  if (protocol_version < 21 &&
      get_channel_relations(relation_with_parent_channel).size()) {
    return false;
  }

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITY:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
    case SUPLA_CHANNELFNC_MAILSENSOR:
    case SUPLA_CHANNELFNC_WINDSENSOR:
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
    case SUPLA_CHANNELFNC_RAINSENSOR:
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
    case SUPLA_CHANNELFNC_WEATHER_STATION:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
    case SUPLA_CHANNELFNC_DIGIGLASS_HORIZONTAL:
    case SUPLA_CHANNELFNC_DIGIGLASS_VERTICAL:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:

      return true;
  }

  if (protocol_version >= 21) {
    switch (Func) {
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_AUTO:
        return true;
    }
  }

  return Type == SUPLA_CHANNELTYPE_BRIDGE && Func == 0;
}

void supla_client_channel::proto_get_value(TSuplaChannelValue_B *value,
                                           char *online, supla_client *client) {
  bool result = false;

  if (client && client->get_user()) {
    unsigned _supla_int_t validity_time_sec = 0;
    result = client->get_user()->get_channel_value(
        DeviceId, get_id(), value->value, value->sub_value,
        &value->sub_value_type, nullptr, nullptr, online, &validity_time_sec,
        true);
    if (result) {
      setValueValidityTimeSec(validity_time_sec);
    }
  }

  if ((!result || (online && !(*online))) && isValueValidityTimeSet() &&
      getValueValidityTimeUSec() > 0) {
    result = true;
    if (online) {
      *online = true;
    }
    memcpy(value->value, this->value, SUPLA_CHANNELVALUE_SIZE);
  }
}

void supla_client_channel::proto_get_value(TSuplaChannelValue *value,
                                           char *online, supla_client *client) {
  TSuplaChannelValue_B value_b = {};
  proto_get_value(&value_b, online, client);

  memcpy(value->value, value_b.value, SUPLA_CHANNELVALUE_SIZE);
  memcpy(value->sub_value, value_b.sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel));

  channel->Id = get_id();
  channel->Func = Func;
  channel->LocationID = this->LocationId;

  proto_get_value(&channel->value, &channel->online, client);
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_B *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_B));

  channel->Id = get_id();
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = get_flags();

  proto_get_value(&channel->value, &channel->online, client);
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_C *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_C));

  channel->Id = get_id();
  channel->DeviceID = get_device_id();
  channel->Type = this->Type;
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->UserIcon = this->UserIcon;
  channel->ManufacturerID = this->ManufacturerID;
  channel->ProductID = this->ProductID;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = get_flags();

  proto_get_value(&channel->value, &channel->online, client);
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_D *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_D));

  channel->Id = get_id();
  channel->DeviceID = get_device_id();
  channel->Type = this->Type;
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->UserIcon = this->UserIcon;
  channel->ManufacturerID = this->ManufacturerID;
  channel->ProductID = this->ProductID;
  channel->ProtocolVersion = this->ProtocolVersion;
  channel->Flags = get_flags();

  proto_get_value(&channel->value, &channel->online, client);
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannelValue *channel_value,
                                     supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue));
  channel_value->Id = get_id();
  proto_get_value(&channel_value->value, &channel_value->online, client);
}

void supla_client_channel::proto_get(TSC_SuplaChannelValue_B *channel_value,
                                     supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue_B));
  channel_value->Id = get_id();
  proto_get_value(&channel_value->value, &channel_value->online, client);
}

bool supla_client_channel::get_cs_extended_value(
    shared_ptr<supla_device> device, int channel_id,
    TSC_SuplaChannelExtendedValue *cev) {
  supla_channel_extended_value *extended_value = nullptr;
  device->get_channels()->access_channel(
      channel_id, [&extended_value](supla_device_channel *channel) -> void {
        extended_value =
            channel->get_extended_value<supla_channel_extended_value>(false);
      });
  if (extended_value) {
    cev->Id = channel_id;
    extended_value->get_raw_value(&cev->value);
    delete extended_value;
    return true;
  }
  return false;
}

bool supla_client_channel::proto_get(TSC_SuplaChannelExtendedValue *cev,
                                     supla_client *client) {
  if (cev == nullptr) {
    return false;
  }

  memset(cev, 0, sizeof(TSC_SuplaChannelExtendedValue));

  if (client && client->get_user()) {
    bool cev_exists = false;

    int ChannelId = get_id();
    shared_ptr<supla_device> device =
        client->get_user()->get_devices()->get(DeviceId);

    if (device != nullptr) {
      cev_exists = get_cs_extended_value(device, ChannelId, cev);
    }

    device = nullptr;
    ChannelId = 0;

    switch (get_func()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
        ChannelId = Param1;  // Associated measurement channel
        break;

      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        ChannelId = Param2;  // Associated measurement channel
        break;
    }

    if (ChannelId) {
      device = client->get_user()->get_devices()->get(0, ChannelId);

      if (device != nullptr) {
        TSC_SuplaChannelExtendedValue second_cev = {};
        if (get_cs_extended_value(device, ChannelId, &second_cev)) {
          if (client->get_protocol_version() >= 17) {
            srpc_evtool_value_add(&cev->value, &second_cev.value);
          } else {
            // For backward compatibility, overwrite cev->value
            memcpy(&cev->value, &second_cev.value,
                   sizeof(TSuplaChannelExtendedValue));
          }

          cev_exists = true;
        }
      }
    }

    if (cev_exists) {
      cev->Id = get_id();
      return true;
    }
  }

  return false;
}

void supla_client_channel::mark_for_remote_update(int mark) {
  supla_client_objcontainer_item::mark_for_remote_update(mark);
  lock();
  mark = marked_for_remote_update();
  if ((mark & OI_REMOTEUPDATE_DATA1) && (mark & OI_REMOTEUPDATE_DATA2)) {
    unmark_for_remote_update(OI_REMOTEUPDATE_DATA2);
  }
  unlock();
}

bool supla_client_channel::get_basic_cfg(TSC_ChannelBasicCfg *basic_cfg) {
  if (basic_cfg == nullptr) return false;

  bool result = false;
  database *db = new database();
  result = db->connect() && db->get_channel_basic_cfg(get_id(), basic_cfg);
  delete db;

  return result;
}

int supla_client_channel::get_channel_id(unsigned char channel_number) {
  return dynamic_cast<supla_client_channels *>(getContainer())
      ->get_channel_id(channel_number);
}

void supla_client_channel::for_each(
    std::function<void(supla_abstract_common_channel_properties *, bool *)>
        on_channel_properties) {
  dynamic_cast<supla_client_channels *>(getContainer())
      ->for_each(
          [&](supla_client_channel *channel, bool *will_continue) -> void {
            on_channel_properties(channel, will_continue);
          });
}
