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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../database.h"
#include "../log.h"
#include "../safearray.h"
#include "../srpc.h"
#include "devicechannel.h"

channel_address::channel_address(int DeviceId, int ChannelId) {
  this->DeviceId = DeviceId;
  this->ChannelId = ChannelId;
}

int channel_address::getDeviceId(void) { return this->DeviceId; }

int channel_address::getChannelId(void) { return this->ChannelId; }

char supla_channel_tarr_clean(void *ptr) {
  delete (supla_channel_temphum *)ptr;
  return 1;
}

supla_channel_temphum::supla_channel_temphum(bool TempAndHumidity,
                                             int ChannelId, double Temperature,
                                             double Humidity) {
  this->ChannelId = ChannelId;
  this->TempAndHumidity = TempAndHumidity;
  this->Temperature = Temperature;
  this->Humidity = Humidity;

  if (this->Temperature < -273 || this->Temperature > 1000) {
    this->Temperature = -273;
  }

  if (this->Humidity < -1 || this->Humidity > 100) {
    this->Humidity = -1;
  }
}

supla_channel_temphum::supla_channel_temphum(
    bool TempAndHumidity, int ChannelId, char value[SUPLA_CHANNELVALUE_SIZE]) {
  this->ChannelId = ChannelId;
  this->TempAndHumidity = TempAndHumidity;
  this->Temperature = -273;
  this->Humidity = -1;

  if (TempAndHumidity) {
    int n;

    memcpy(&n, value, 4);
    this->Temperature = n / 1000.00;

    memcpy(&n, &value[4], 4);
    this->Humidity = n / 1000.00;

  } else {
    memcpy(&this->Temperature, value, sizeof(double));
  }

  supla_channel_temphum(TempAndHumidity, ChannelId, this->Temperature,
                        this->Humidity);
}

int supla_channel_temphum::getChannelId(void) { return ChannelId; }

bool supla_channel_temphum::isTempAndHumidity(void) { return TempAndHumidity; }

double supla_channel_temphum::getTemperature(void) { return Temperature; }

double supla_channel_temphum::getHumidity(void) { return Humidity; }

void supla_channel_temphum::setTemperature(double Temperature) {
  this->Temperature = Temperature;
}

void supla_channel_temphum::setHumidity(double Humidity) {
  this->Humidity = Humidity;
}

void supla_channel_temphum::toValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

  if (isTempAndHumidity()) {
    int n;

    n = this->Temperature * 1000;
    memcpy(value, &n, 4);

    n = this->Humidity * 1000;
    memcpy(&value[4], &n, 4);

  } else {
    memcpy(value, &this->Temperature, sizeof(double));
  }
}

void supla_channel_temphum::free(void *tarr) {
  safe_array_clean(tarr, supla_channel_tarr_clean);
  safe_array_free(tarr);
}

//-----------------------------------------------------

supla_device_channel::supla_device_channel(int Id, int Number, int Type,
                                           int Func, int Param1, int Param2,
                                           int Param3, bool Hidden) {
  this->Id = Id;
  this->Number = Number;
  this->Type = Type;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->Param3 = Param3;
  this->Hidden = Hidden;

  memset(this->value, 0, SUPLA_CHANNELVALUE_SIZE);
}

int supla_device_channel::getId(void) { return Id; }

int supla_device_channel::getNumber(void) { return Number; }

int supla_device_channel::getFunc(void) { return Func; }

int supla_device_channel::getType(void) { return Type; }

int supla_device_channel::getParam1(void) { return Param1; }

bool supla_device_channel::getHidden(void) { return Hidden; }

void supla_device_channel::getValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
}

void supla_device_channel::getDouble(double *Value) {
  if (Value == NULL) return;

  switch (Type) {
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_SENSORNC:
      *Value = this->value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
      memcpy(Value, this->value, sizeof(double));
      break;
    default:
      *Value = 0;
  }
}

void supla_device_channel::getChar(char *Value) {
  if (Value == NULL) return;
  *Value = this->value[0];
}

bool supla_device_channel::getRGBW(int *color, char *color_brightness,
                                   char *brightness) {
  if (color != NULL) *color = 0;

  if (color_brightness != NULL) *color_brightness = 0;

  if (brightness != NULL) *brightness = 0;

  bool result = false;

  if (Type == SUPLA_CHANNELTYPE_DIMMER ||
      Type == SUPLA_CHANNELTYPE_DIMMERANDRGBLED) {
    if (brightness != NULL) {
      *brightness = this->value[0];

      if (*brightness < 0 || *brightness > 100) *brightness = 0;
    }

    result = true;
  }

  if (Type == SUPLA_CHANNELTYPE_RGBLEDCONTROLLER ||
      Type == SUPLA_CHANNELTYPE_DIMMERANDRGBLED) {
    if (color_brightness != NULL) {
      *color_brightness = this->value[1];

      if (*color_brightness < 0 || *color_brightness > 100)
        *color_brightness = 0;
    }

    if (color != NULL) {
      *color = 0;

      *color = this->value[4] & 0xFF;
      (*color) <<= 8;

      *color |= this->value[3] & 0xFF;
      (*color) <<= 8;

      (*color) |= this->value[2] & 0xFF;
    }

    result = true;
  }

  return result;
}

void supla_device_channel::setValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  if (Type == SUPLA_CHANNELTYPE_SENSORNC) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;

  } else {
    supla_channel_temphum *TempHum = getTempHum();

    if (TempHum) {
      if ((Param2 != 0 || Param3 != 0)) {
        if (Param2 != 0) {
          TempHum->setTemperature(TempHum->getTemperature() +
                                  (Param2 / 100.00));
        }

        if (Param3 != 0) {
          TempHum->setHumidity(TempHum->getHumidity() + (Param3 / 100.00));
        }

        TempHum->toValue(this->value);
      }

      delete TempHum;
    }
  }

  if (Param3 == 1 && (Type == SUPLA_CHANNELTYPE_SENSORNC ||
                      Type == SUPLA_CHANNELTYPE_SENSORNO)) {
    this->value[0] = this->value[0] == 0 ? 1 : 0;
  }
}

void supla_device_channel::assignRgbwValue(char value[SUPLA_CHANNELVALUE_SIZE],
                                           int color, char color_brightness,
                                           char brightness) {
  if (Func == SUPLA_CHANNELFNC_DIMMER ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (brightness < 0 || brightness > 100) brightness = 0;

    value[0] = brightness;
  }

  if (Func == SUPLA_CHANNELFNC_RGBLIGHTING ||
      Func == SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING) {
    if (color_brightness < 0 || color_brightness > 100) color_brightness = 0;

    value[1] = color_brightness;
    value[2] = (char)((color & 0x000000FF));
    value[3] = (char)((color & 0x0000FF00) >> 8);
    value[4] = (char)((color & 0x00FF0000) >> 16);
  }
}

void supla_device_channel::assignCharValue(char value[SUPLA_CHANNELVALUE_SIZE],
                                           char cvalue) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
  value[0] = cvalue;
}

bool supla_device_channel::isValueWritable(void) {
  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 1;

      break;
  }

  return 0;
}

bool supla_device_channel::isCharValueWritable(void) {
  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      return 1;

      break;
  }

  return 0;
}

bool supla_device_channel::isRgbwValueWritable(void) {
  switch (Func) {
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return 1;

      break;
  }

  return 0;
}

unsigned int supla_device_channel::getValueDuration(void) {
  assert(sizeof(int) == 4 && sizeof(short) == 2);

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return Param1;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER: {
      unsigned int result = 0;

      result = (unsigned short)Param1;
      result <<= 16;
      result |= (unsigned short)Param3;

      return result;
    }
  }

  return 0;
}

std::list<int> supla_device_channel::slave_channel(void) {
  std::list<int> result;

  switch (Func) {
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:

      // The order is important !!!
      // 1st Param2
      // 2nd Param3

      if (Param2) {
        result.push_back(Param2);
      }

      if (Param3) {
        result.push_back(Param3);
      }

      break;
  }

  return result;
}

std::list<int> supla_device_channel::master_channel(void) {
  std::list<int> result;

  switch (Func) {
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:

      if (Param1) {
        result.push_back(Param1);
      }

      if (Param2) {
        result.push_back(Param2);
      }

      break;
  }

  return result;
}

supla_channel_temphum *supla_device_channel::getTempHum(void) {
  double temp;

  if (getType() == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 &&
      getFunc() == SUPLA_CHANNELFNC_THERMOMETER) {
    getDouble(&temp);

    if (temp > -273 && temp <= 1000) {
      return new supla_channel_temphum(0, getId(), value);
    }

  } else if ((getType() == SUPLA_CHANNELTYPE_DHT11 ||
              getType() == SUPLA_CHANNELTYPE_DHT22 ||
              getType() == SUPLA_CHANNELTYPE_DHT21 ||
              getType() == SUPLA_CHANNELTYPE_AM2301 ||
              getType() == SUPLA_CHANNELTYPE_AM2302) &&
             (getFunc() == SUPLA_CHANNELFNC_THERMOMETER ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITY ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE)) {
    int n;
    char value[SUPLA_CHANNELVALUE_SIZE];
    double humidity;

    getValue(value);
    memcpy(&n, value, 4);
    temp = n / 1000.00;

    memcpy(&n, &value[4], 4);
    humidity = n / 1000.00;

    if (temp > -273 && temp <= 1000 && humidity >= 0 && humidity <= 100) {
      return new supla_channel_temphum(1, getId(), value);
    }
  }

  return NULL;
}

// ---------------------------------------------
// ---------------------------------------------
// ---------------------------------------------

supla_device_channels::supla_device_channels() { arr = safe_array_init(); }

supla_device_channels::~supla_device_channels() {
  arr_clean();
  safe_array_free(arr);
}

char supla_device_channels::arr_findcmp(void *ptr, void *id) {
  return ((supla_device_channel *)ptr)->getId() == *((int *)id) ? 1 : 0;
}

char supla_device_channels::arr_findncmp(void *ptr, void *n) {
  return ((supla_device_channel *)ptr)->getNumber() == *((int *)n) ? 1 : 0;
}

char supla_device_channels::arr_delcnd(void *ptr) {
  delete (supla_device_channel *)ptr;
  return 1;
}

void supla_device_channels::arr_clean(void) {
  safe_array_lock(arr);
  safe_array_clean(arr, arr_delcnd);
  safe_array_unlock(arr);
}

supla_device_channel *supla_device_channels::find_channel(int Id) {
  return (supla_device_channel *)safe_array_findcnd(arr, arr_findcmp, &Id);
}

supla_device_channel *supla_device_channels::find_channel_by_number(
    int Number) {
  return (supla_device_channel *)safe_array_findcnd(arr, arr_findncmp, &Number);
}

void supla_device_channels::add_channel(int Id, int Number, int Type, int Func,
                                        int Param1, int Param2, int Param3,
                                        bool Hidden) {
  safe_array_lock(arr);

  if (find_channel(Id) == 0) {
    supla_device_channel *c = new supla_device_channel(
        Id, Number, Type, Func, Param1, Param2, Param3, Hidden);

    if (c != NULL && safe_array_add(arr, c) == -1) {
      delete c;
      c = NULL;
    }
  }

  safe_array_unlock(arr);
}

void supla_device_channels::load(int DeviceID) {
  database *db = new database();

  if (db->connect() == true) {
    safe_array_lock(arr);
    arr_clean();

    db->get_device_channels(DeviceID, this);

    safe_array_unlock(arr);
  }

  delete db;
}

bool supla_device_channels::get_channel_value(
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getValue(value);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_double_value(int ChannelID,
                                                     double *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getDouble(Value);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

supla_channel_temphum *
supla_device_channels::get_channel_temp_and_humidity_value(int ChannelID) {
  supla_channel_temphum *result = NULL;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      result = channel->getTempHum();
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_temperature_value(int ChannelID,
                                                          double *Value) {
  supla_channel_temphum *result =
      get_channel_temp_and_humidity_value(ChannelID);
  if (result) {
    *Value = result->getTemperature();
    delete result;
    return true;
  }

  return false;
}

bool supla_device_channels::get_channel_humidity_value(int ChannelID,
                                                       double *Value) {
  supla_channel_temphum *result =
      get_channel_temp_and_humidity_value(ChannelID);
  if (result) {
    if (result->isTempAndHumidity() == 1) {
      *Value = result->getHumidity();
      delete result;
      return true;

    } else {
      delete result;
    }
  }

  return false;
}

bool supla_device_channels::get_channel_char_value(int ChannelID, char *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getChar(Value);
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

void supla_device_channels::set_channel_value(
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE]) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) channel->setValue(value);

  safe_array_unlock(arr);
}

unsigned int supla_device_channels::get_channel_value_duration(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Duration = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Duration = channel->getValueDuration();

  safe_array_unlock(arr);

  return Duration;
}

int supla_device_channels::get_channel_func(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Func = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Func = channel->getFunc();

  safe_array_unlock(arr);

  return Func;
}

std::list<int> supla_device_channels::ms_channel(int ChannelID, bool Master) {
  std::list<int> result;
  if (ChannelID == 0) return result;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel)
    result = Master ? channel->master_channel() : channel->slave_channel();

  safe_array_unlock(arr);

  return result;
}

std::list<int> supla_device_channels::master_channel(int ChannelID) {
  return ms_channel(ChannelID, true);
}

std::list<int> supla_device_channels::slave_channel(int ChannelID) {
  return ms_channel(ChannelID, false);
}

bool supla_device_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::set_channels_value(
    TDS_SuplaDeviceChannel_B *schannel, int count) {
  for (int a = 0; a < count; a++)
    set_channel_value(get_channel_id(schannel[a].Number), schannel[a].value);
}

int supla_device_channels::get_channel_id(unsigned char ChannelNumber) {
  int result = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel_by_number(ChannelNumber);

  if (channel) result = channel->getId();

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::set_device_channel_value(
    void *srpc, int SenderID, int ChannelID,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isValueWritable()) {
    TSD_SuplaChannelNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

    s.ChannelNumber = channel->getNumber();
    s.DurationMS = channel->getValueDuration();
    s.SenderID = SenderID;
    memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

    supla_log(LOG_DEBUG, "Set channel value %i", ChannelID);
    srpc_sd_async_set_channel_value(srpc, &s);
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::set_device_channel_char_value(void *srpc,
                                                          int SenderID,
                                                          int ChannelID,
                                                          const char value) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isCharValueWritable()) {
    TSD_SuplaChannelNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

    s.ChannelNumber = channel->getNumber();
    s.DurationMS = channel->getValueDuration();
    s.SenderID = SenderID;

    channel->assignCharValue(s.value, value);

    srpc_sd_async_set_channel_value(srpc, &s);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::set_device_channel_rgbw_value(
    void *srpc, int SenderID, int ChannelID, int color, char color_brightness,
    char brightness) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isRgbwValueWritable()) {
    TSD_SuplaChannelNewValue s;
    memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

    s.ChannelNumber = channel->getNumber();
    s.DurationMS = channel->getValueDuration();
    s.SenderID = SenderID;

    channel->assignRgbwValue(s.value, color, color_brightness, brightness);

    srpc_sd_async_set_channel_value(srpc, &s);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_temp_and_humidity(void *tarr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL) {
      supla_channel_temphum *temphum = channel->getTempHum();

      if (temphum != NULL) safe_array_add(tarr, temphum);
    }
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::get_channel_rgbw_value(int ChannelID, int *color,
                                                   char *color_brightness,
                                                   char *brightness) {
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    int _color;
    char _color_brightness;
    char _brightness;

    result = channel->getRGBW(&_color, &_color_brightness, &_brightness);

    if (result == true) {
      if (color != NULL) *color = _color;

      if (color_brightness) *color_brightness = _color_brightness;

      if (brightness != NULL) *brightness = _brightness;
    }
  }

  safe_array_unlock(arr);

  return result;
}
