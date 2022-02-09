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

#include "channel_temphum.h"

#include <string.h>

#include "safearray.h"

supla_channel_temphum::supla_channel_temphum(bool TempAndHumidity,
                                             int ChannelId, double Temperature,
                                             double Humidity) {
  this->ChannelId = ChannelId;
  this->TempAndHumidity = TempAndHumidity;

  setTemperature(Temperature);
  setHumidity(Humidity);
}

supla_channel_temphum::supla_channel_temphum(
    bool TempAndHumidity, int ChannelId,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  this->ChannelId = ChannelId;
  this->TempAndHumidity = TempAndHumidity;
  this->Temperature = -273;
  this->Humidity = -1;

  assignValue(value, TempAndHumidity);
}

supla_channel_temphum::supla_channel_temphum(
    int ChannelId, int Func, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  this->ChannelId = ChannelId;
  this->TempAndHumidity = Func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;
  this->Temperature = -273;
  this->Humidity = -1;

  assignValue(value, TempAndHumidity);
}

int supla_channel_temphum::getChannelId(void) { return ChannelId; }

bool supla_channel_temphum::isTempAndHumidity(void) { return TempAndHumidity; }

double supla_channel_temphum::getTemperature(void) { return Temperature; }

double supla_channel_temphum::getHumidity(void) { return Humidity; }

void supla_channel_temphum::setTemperature(double Temperature) {
  if (Temperature < -273 || Temperature > 1000) {
    Temperature = -273;
  }

  this->Temperature = Temperature;
}

void supla_channel_temphum::setHumidity(double Humidity) {
  if (Humidity < -1 || Humidity > 100) {
    Humidity = -1;
  }

  this->Humidity = Humidity;
}

void supla_channel_temphum::assignValue(
    const char value[SUPLA_CHANNELVALUE_SIZE], bool TempAndHumidity) {
  if (TempAndHumidity) {
    int n;

    memcpy(&n, value, 4);
    setTemperature(n / 1000.00);

    memcpy(&n, &value[4], 4);
    setHumidity(n / 1000.00);

  } else {
    double Temperature;
    memcpy(&Temperature, value, sizeof(double));
    setTemperature(Temperature);
  }
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

// static
char supla_channel_temphum::tarr_clean(void *ptr) {
  delete (supla_channel_temphum *)ptr;
  return 1;
}

// static
void supla_channel_temphum::free(void *tarr) {
  safe_array_clean(tarr, tarr_clean);
  safe_array_free(tarr);
}
