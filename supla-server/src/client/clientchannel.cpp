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
#include "jsonconfig/channel/electricity_meter_config.h"
#include "jsonconfig/channel/hvac_config.h"
#include "jsonconfig/channel/power_switch_config.h"
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
    short ManufacturerID, short ProductID, unsigned char DeviceProtocolVersion,
    unsigned _supla_int64_t Flags, const char value[SUPLA_CHANNELVALUE_SIZE],
    unsigned _supla_int_t validity_time_sec, const char *user_config,
    const char *properties)
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
  this->DeviceProtocolVersion = DeviceProtocolVersion;
  this->Flags = Flags;
  this->json_config = nullptr;

  supla_json_config *json_config = new supla_json_config();
  json_config->set_user_config(user_config);
  json_config->set_properties(properties);
  set_json_config(json_config);

  if (Type == SUPLA_CHANNELTYPE_ELECTRICITY_METER) {
    electricity_meter_config *config = new electricity_meter_config();

    config->set_user_config(user_config);
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

void supla_client_channel::after_all_channels_loaded(void) {
  unsigned _supla_int64_t Flags = 0;
  switch (get_func()) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER: {
      int meter_channel_id = 0;
      supla_json_config *json_config = get_json_config();
      if (json_config) {
        power_switch_config config(json_config);
        meter_channel_id = config.get_related_meter_channel_id();
        delete json_config;
      }

      if (meter_channel_id) {
        for_each(true,
                 [meter_channel_id, &Flags](
                     supla_abstract_common_channel_properties *props,
                     bool *will_continue) -> void {
                   if (props->get_id() == meter_channel_id) {
                     int F = props->get_flags();
                     Flags |= F & SUPLA_CHANNEL_FLAG_PHASE1_UNSUPPORTED;
                     Flags |= F & SUPLA_CHANNEL_FLAG_PHASE2_UNSUPPORTED;
                     Flags |= F & SUPLA_CHANNEL_FLAG_PHASE3_UNSUPPORTED;

                     *will_continue = false;
                   }
                 });
      }
    } break;
  }

  if (Flags) {
    this->Flags |= Flags;
  }
}

unsigned char supla_client_channel::get_channel_number(void) {
  return channel_number;
}

int supla_client_channel::get_user_id() {
  return dynamic_cast<supla_client_channels *>(getContainer())
      ->getClient()
      ->get_user_id();
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

supla_json_config *supla_client_channel::get_json_config(void) {
  lock();
  supla_json_config *result = new supla_json_config(json_config, true);
  unlock();

  return result;
}

void supla_client_channel::set_json_config(supla_json_config *json_config) {
  lock();
  if (this->json_config) {
    delete this->json_config;
  }

  this->json_config = json_config;
  unlock();
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

unsigned _supla_int64_t supla_client_channel::get_flags() {
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
        (value_valid_to.tv_sec * 1000000LL + value_valid_to.tv_usec) -
        (now.tv_sec * 1000000LL + now.tv_usec);
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
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
    case SUPLA_CHANNELFNC_TERRACE_AWNING:
    case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
    case SUPLA_CHANNELFNC_CURTAIN:
    case SUPLA_CHANNELFNC_VERTICAL_BLIND:
    case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR:
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
    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
    case SUPLA_CHANNELFNC_ALARMARMAMENTSENSOR:
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
    case SUPLA_CHANNELFNC_IC_GAS_METER:
    case SUPLA_CHANNELFNC_IC_WATER_METER:
    case SUPLA_CHANNELFNC_IC_HEAT_METER:

      return true;
  }

  if (protocol_version >= 21) {
    switch (Func) {
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
      case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
      case SUPLA_CHANNELFNC_GENERAL_PURPOSE_MEASUREMENT:
      case SUPLA_CHANNELFNC_GENERAL_PURPOSE_METER:
        return true;
    }
  }

  if (protocol_version >= 25) {
    switch (Func) {
      case SUPLA_CHANNELFNC_PUMPSWITCH:
      case SUPLA_CHANNELFNC_HEATORCOLDSOURCESWITCH:
        return true;
    }
  }

  if (protocol_version >= 26) {
    switch (Func) {
      case SUPLA_CHANNELFNC_CONTAINER:
      case SUPLA_CHANNELFNC_CONTAINER_LEVEL_SENSOR:
        return true;
    }
  }

  return Type == SUPLA_CHANNELTYPE_BRIDGE && Func == 0;
}

void supla_client_channel::proto_get_value(
    TSuplaChannelValue_B *value, supla_channel_availability_status *status,
    supla_client *client) {
  bool result = false;

  if (client && client->get_user()) {
    unsigned _supla_int_t validity_time_sec = 0;
    result = client->get_user()->get_channel_value(
        DeviceId, get_id(), value->value, value->sub_value,
        &value->sub_value_type, nullptr, nullptr, status, &validity_time_sec,
        true);
    if (result) {
      setValueValidityTimeSec(validity_time_sec);
      memcpy(this->value, value->value, SUPLA_CHANNELVALUE_SIZE);
    }
  }

  if ((!result || (status && !status->is_online())) &&
      isValueValidityTimeSet() && getValueValidityTimeUSec() > 0) {
    result = true;
    if (status) {
      status->set_offline(false);
    }
    memcpy(value->value, this->value, SUPLA_CHANNELVALUE_SIZE);
  }
}

void supla_client_channel::proto_get_value(
    TSuplaChannelValue *value, supla_channel_availability_status *status,
    supla_client *client) {
  TSuplaChannelValue_B value_b = {};
  proto_get_value(&value_b, status, client);

  memcpy(value->value, value_b.value, SUPLA_CHANNELVALUE_SIZE);
  memcpy(value->sub_value, value_b.sub_value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel));

  channel->Id = get_id();
  channel->Func = Func;
  channel->LocationID = this->LocationId;

  supla_channel_availability_status status(true);

  proto_get_value(&channel->value, &status, client);
  channel->online = status.get_proto_online();
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
  channel->ProtocolVersion = this->DeviceProtocolVersion;
  channel->Flags = get_flags() & 0xFFFFFFFF;

  supla_channel_availability_status status(true);

  proto_get_value(&channel->value, &status, client);
  channel->online = status.get_proto_online();
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
  channel->ProtocolVersion = this->DeviceProtocolVersion;
  channel->Flags = get_flags() & 0xFFFFFFFF;

  supla_channel_availability_status status(true);

  proto_get_value(&channel->value, &status, client);
  channel->online = status.get_proto_online();
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
  channel->ProtocolVersion = this->DeviceProtocolVersion;
  channel->Flags = get_flags() & 0xFFFFFFFF;

  supla_channel_availability_status status(true);

  proto_get_value(&channel->value, &status, client);
  channel->online = status.get_proto_online();
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannel_E *channel,
                                     supla_client *client) {
  memset(channel, 0, sizeof(TSC_SuplaChannel_E));

  channel->Id = get_id();
  channel->DeviceID = get_device_id();
  channel->Type = this->Type;
  channel->Func = Func;
  channel->LocationID = this->LocationId;
  channel->AltIcon = this->AltIcon;
  channel->UserIcon = this->UserIcon;
  channel->ManufacturerID = this->ManufacturerID;
  channel->ProductID = this->ProductID;
  channel->ProtocolVersion = this->DeviceProtocolVersion;
  channel->Flags = get_flags();

  TSCS_ChannelConfig config = {};
  get_config(&config, SUPLA_CONFIG_TYPE_DEFAULT, nullptr, 0);

  channel->DefaultConfigCRC32 = st_crc32_checksum(
      (const unsigned char *)config.Config, config.ConfigSize);

  supla_channel_availability_status status(true);

  proto_get_value(&channel->value, &status, client);
  channel->online = status.get_proto_online();
  sproto_set_null_terminated_string(getCaption(), channel->Caption,
                                    &channel->CaptionSize,
                                    SUPLA_CHANNEL_CAPTION_MAXSIZE);
}

void supla_client_channel::proto_get(TSC_SuplaChannelValue *channel_value,
                                     supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue));
  channel_value->Id = get_id();

  supla_channel_availability_status status(true);

  proto_get_value(&channel_value->value, &status, client);
  channel_value->online = status.get_proto_online();
}

void supla_client_channel::proto_get(TSC_SuplaChannelValue_B *channel_value,
                                     supla_client *client) {
  memset(channel_value, 0, sizeof(TSC_SuplaChannelValue_B));
  channel_value->Id = get_id();

  supla_channel_availability_status status(true);

  proto_get_value(&channel_value->value, &status, client);
  channel_value->online = status.get_proto_online();
}

bool supla_client_channel::get_cs_extended_value(
    shared_ptr<supla_device> device, int channel_id,
    TSC_SuplaChannelExtendedValue *cev, unsigned char protocol_version) {
  supla_channel_extended_value *extended_value = nullptr;
  device->get_channels()->access_channel(
      channel_id, [&extended_value](supla_device_channel *channel) -> void {
        extended_value =
            channel->get_extended_value<supla_channel_extended_value>(false);
      });
  if (extended_value) {
    cev->Id = channel_id;
    extended_value->get_raw_value(&cev->value, protocol_version);
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
      cev_exists = get_cs_extended_value(device, ChannelId, cev,
                                         client->get_protocol_version());
    }

    device = nullptr;
    ChannelId = 0;

    switch (get_func()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER: {
        supla_json_config *json_config = get_json_config();
        if (json_config) {
          power_switch_config config(json_config);
          ChannelId = config.get_related_meter_channel_id();
          delete json_config;
        }
      } break;
    }

    if (ChannelId) {
      device = client->get_user()->get_devices()->get(0, ChannelId);

      if (device != nullptr) {
        TSC_SuplaChannelExtendedValue second_cev = {};
        if (get_cs_extended_value(device, ChannelId, &second_cev,
                                  client->get_protocol_version())) {
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

void supla_client_channel::for_each(
    bool any_device,
    std::function<void(supla_abstract_common_channel_properties *, bool *)>
        on_channel_properties) {
  dynamic_cast<supla_client_channels *>(getContainer())
      ->for_each(
          [&](supla_client_channel *channel, bool *will_continue) -> void {
            if (any_device || get_device_id() == channel->get_device_id()) {
              on_channel_properties(channel, will_continue);
            }
          });
}

unsigned char supla_client_channel::get_real_config_type(
    unsigned char config_type) {
  if ((config_type == SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE ||
       config_type == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) &&
      get_func() == SUPLA_CHANNELFNC_HVAC_THERMOSTAT) {
    TChannelConfig_HVAC native_cfg = {};
    lock();
    hvac_config hvac_cfg(json_config);
    hvac_cfg.get_config(&native_cfg, get_channel_number());
    unlock();

    if (native_cfg.Subfunction == SUPLA_HVAC_SUBFUNCTION_COOL) {
      config_type = SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE;
    }
  }

  return config_type;
}

void supla_client_channel::get_config(TSCS_ChannelConfig *config,
                                      unsigned char config_type,
                                      unsigned char *real_config_type,
                                      unsigned _supla_int_t flags) {
  unsigned char rct = config_type;

  if (config) {
    rct = get_real_config_type(config_type);

    supla_abstract_common_channel_properties::get_config(
        config->Config, &config->ConfigSize, rct, flags, true);

    config->ChannelId = get_id();
    config->ConfigType = config_type;
    config->Func = get_func();

    if (config->ConfigType == SUPLA_CONFIG_TYPE_ALT_WEEKLY_SCHEDULE) {
      config->ConfigType = SUPLA_CONFIG_TYPE_WEEKLY_SCHEDULE;
    }
  }

  if (real_config_type) {
    *real_config_type = rct;
  }
}

int supla_client_channel::set_user_config(unsigned char config_type,
                                          unsigned _supla_int16_t config_size,
                                          char *config) {
  return supla_abstract_common_channel_properties::set_user_config(
      get_real_config_type(config_type), config_size, config);
}

unsigned char supla_client_channel::get_protocol_version(void) {
  supla_client_channels *channels =
      dynamic_cast<supla_client_channels *>(getContainer());
  if (channels && channels->getClient() &&
      channels->getClient()->get_connection()) {
    return channels->getClient()->get_connection()->get_protocol_version();
  }

  return 0;
}
