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

#include "database.h"
#include "devicechannel.h"
#include "log.h"
#include "safearray.h"
#include "srpc.h"
#include "user/user.h"

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
    bool TempAndHumidity, int ChannelId,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  // Because of linter
  this->ChannelId = ChannelId;
  this->TempAndHumidity = TempAndHumidity;
  this->Temperature = -273;
  this->Humidity = -1;
  // ----------------

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

supla_channel_temphum::supla_channel_temphum(
    int ChannelId, int Func, const char value[SUPLA_CHANNELVALUE_SIZE]) {
  // Because of linter
  this->ChannelId = ChannelId;
  this->Temperature = -273;
  this->Humidity = -1;
  // ----------------
  this->TempAndHumidity = Func == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE;


  supla_channel_temphum(TempAndHumidity, ChannelId, value);
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

// static
void supla_channel_temphum::free(void *tarr) {
  safe_array_clean(tarr, supla_channel_tarr_clean);
  safe_array_free(tarr);
}

char supla_channel_emarr_clean(void *ptr) {
  delete (supla_channel_electricity_measurement *)ptr;
  return 1;
}

char supla_channel_icarr_clean(void *ptr) {
  delete (supla_channel_ic_measurement *)ptr;
  return 1;
}

char supla_channel_tharr_clean(void *ptr) {
  delete (supla_channel_thermostat_measurement *)ptr;
  return 1;
}

supla_channel_electricity_measurement::supla_channel_electricity_measurement(
    int ChannelId, TElectricityMeter_ExtendedValue *em_ev, int Param2,
    char *TextParam1) {
  this->ChannelId = ChannelId;
  if (em_ev == NULL) {
    assign(Param2, TextParam1, NULL);
  } else {
    TElectricityMeter_ExtendedValue_V2 em_ev_v2;
    srpc_evtool_emev_v1to2(em_ev, &em_ev_v2);
    assign(Param2, TextParam1, &em_ev_v2);
  }
}

supla_channel_electricity_measurement::supla_channel_electricity_measurement(
    int ChannelId, TElectricityMeter_ExtendedValue_V2 *em_ev, int Param2,
    char *TextParam1) {
  this->ChannelId = ChannelId;
  assign(Param2, TextParam1, em_ev);
}

void supla_channel_electricity_measurement::assign(
    int Param2, char *TextParam1, TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (em_ev == NULL) {
    memset(&this->em_ev, 0, sizeof(TElectricityMeter_ExtendedValue));
  } else {
    memcpy(&this->em_ev, em_ev, sizeof(TElectricityMeter_ExtendedValue));

    if (this->em_ev.m_count == 0) {
      memset(this->em_ev.m, 0, sizeof(TElectricityMeter_Measurement));
    }
  }

  supla_channel_electricity_measurement::set_costs(Param2, TextParam1,
                                                   &this->em_ev);
}

int supla_channel_electricity_measurement::getChannelId(void) {
  return ChannelId;
}

void supla_channel_electricity_measurement::getMeasurement(
    TElectricityMeter_ExtendedValue *em_ev) {
  if (em_ev) {
    srpc_evtool_emev_v2to1(&this->em_ev, em_ev);
  }
}

void supla_channel_electricity_measurement::getMeasurement(
    TElectricityMeter_ExtendedValue_V2 *em_ev) {
  if (em_ev) {
    memcpy(em_ev, &this->em_ev, sizeof(TElectricityMeter_ExtendedValue_V2));
  }
}

void supla_channel_electricity_measurement::getCurrency(char currency[4]) {
  memcpy(currency, this->em_ev.currency, 3);
  currency[3] = 0;
}

// static
void supla_channel_electricity_measurement::set_costs(
    int Param2, char *TextParam1, TElectricityMeter_ExtendedValue *em_ev) {
  double sum = em_ev->total_forward_active_energy[0] * 0.00001;
  sum += em_ev->total_forward_active_energy[1] * 0.00001;
  sum += em_ev->total_forward_active_energy[2] * 0.00001;

  supla_channel_ic_measurement::get_cost_and_currency(
      TextParam1, Param2, em_ev->currency, &em_ev->total_cost,
      &em_ev->price_per_unit, sum);
}

// static
void supla_channel_electricity_measurement::set_costs(
    int Param2, char *TextParam1, TElectricityMeter_ExtendedValue_V2 *em_ev) {
  double sum = em_ev->total_forward_active_energy[0] * 0.00001;
  sum += em_ev->total_forward_active_energy[1] * 0.00001;
  sum += em_ev->total_forward_active_energy[2] * 0.00001;

  supla_channel_ic_measurement::get_cost_and_currency(
      TextParam1, Param2, em_ev->currency, &em_ev->total_cost,
      &em_ev->price_per_unit, sum);

  if (em_ev->measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED) {
    supla_channel_ic_measurement::get_cost_and_currency(
        TextParam1, Param2, em_ev->currency, &em_ev->total_cost_balanced,
        &em_ev->price_per_unit,
        em_ev->total_forward_active_energy_balanced * 0.00001);
  } else {
    em_ev->total_cost_balanced = 0;
  }
}

// static
bool supla_channel_electricity_measurement::update_cev(
    TSC_SuplaChannelExtendedValue *cev, int Param2, char *TextParam1,
    bool convert_to_v1) {
  if (cev->value.type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1) {
    TElectricityMeter_ExtendedValue em_ev;
    if (srpc_evtool_v1_extended2emextended(&cev->value, &em_ev)) {
      supla_channel_electricity_measurement::set_costs(Param2, TextParam1,
                                                       &em_ev);
      srpc_evtool_v1_emextended2extended(&em_ev, &cev->value);
      return true;
    }
  } else if (cev->value.type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
    TElectricityMeter_ExtendedValue_V2 em_ev;
    if (srpc_evtool_v2_extended2emextended(&cev->value, &em_ev)) {
      supla_channel_electricity_measurement::set_costs(Param2, TextParam1,
                                                       &em_ev);
      if (convert_to_v1) {
        TElectricityMeter_ExtendedValue em_ev_v1;
        srpc_evtool_emev_v2to1(&em_ev, &em_ev_v1);
        srpc_evtool_v1_emextended2extended(&em_ev_v1, &cev->value);
      } else {
        srpc_evtool_v2_emextended2extended(&em_ev, &cev->value);
      }

      return true;
    }
  }

  return false;
}

// static
void supla_channel_electricity_measurement::free(void *emarr) {
  safe_array_clean(emarr, supla_channel_emarr_clean);
  safe_array_free(emarr);
}

//-----------------------------------------------------

supla_channel_ic_measurement::supla_channel_ic_measurement(
    int ChannelId, int Func, TDS_ImpulseCounter_Value *ic_val, char *TextParam1,
    char *TextParam2, int Param2, int Param3) {
  this->ChannelId = ChannelId;
  this->totalCost = 0;
  this->pricePerUnit = 0;
  this->currency[0] = 0;
  this->impulsesPerUnit = 0;
  this->customUnit[0] = 0;

  if (TextParam2 && strnlen(TextParam2, 9) < 9) {
    strncpy(this->customUnit, TextParam2, 9);
    this->customUnit[8] = 0;
  }

  supla_channel_ic_measurement::set_default_unit(Func, this->customUnit);

  if (Param3 > 0) {
    this->impulsesPerUnit = Param3;
  }

  this->counter = ic_val->counter;

  this->calculatedValue = supla_channel_ic_measurement::get_calculated_i(
      this->impulsesPerUnit, this->counter);

  supla_channel_ic_measurement::get_cost_and_currency(
      TextParam1, Param2, this->currency, &this->totalCost, &this->pricePerUnit,
      supla_channel_ic_measurement::get_calculated_d(this->impulsesPerUnit,
                                                     this->counter));

  this->currency[3] = 0;
}

int supla_channel_ic_measurement::getChannelId(void) { return ChannelId; }

_supla_int_t supla_channel_ic_measurement::getTotalCost(void) {
  return totalCost;
}

_supla_int_t supla_channel_ic_measurement::getPricePerUnit(void) {
  return pricePerUnit;
}

const char *supla_channel_ic_measurement::getCurrncy(void) { return currency; }

const char *supla_channel_ic_measurement::getCustomUnit(void) {
  return customUnit;
}

_supla_int_t supla_channel_ic_measurement::getImpulsesPerUnit(void) {
  return impulsesPerUnit;
}

unsigned _supla_int64_t supla_channel_ic_measurement::getCounter(void) {
  return counter;
}

unsigned _supla_int64_t supla_channel_ic_measurement::getCalculatedValue(void) {
  return calculatedValue;
}

// static
void supla_channel_ic_measurement::set_default_unit(int Func, char unit[9]) {
  if (strnlen(unit, 9) == 0) {
    switch (Func) {
      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        snprintf(unit, 9, "kWh");  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        // UTF(³) == 0xc2b3
        snprintf(unit, 9, "m%c%c", 0xc2, 0xb3);  // NOLINT
        break;
      case SUPLA_CHANNELFNC_IC_HEAT_METER:
        snprintf(unit, 9, "GJ");  // NOLINT
        break;
    }
  }
}

// static
bool supla_channel_ic_measurement::update_cev(
    TSC_SuplaChannelExtendedValue *cev, int Func, int Param2, int Param3,
    char *TextParam1, char *TextParam2) {
  TSC_ImpulseCounter_ExtendedValue ic_ev;
  if (srpc_evtool_v1_extended2icextended(&cev->value, &ic_ev)) {
    ic_ev.calculated_value = 0;
    ic_ev.custom_unit[0] = 0;
    ic_ev.impulses_per_unit = 0;

    if (TextParam2 && strnlen(TextParam2, 9) < 9) {
      strncpy(ic_ev.custom_unit, TextParam2, 8);
      ic_ev.custom_unit[8] = 0;
    }

    supla_channel_ic_measurement::set_default_unit(Func, ic_ev.custom_unit);

    if (Param3 > 0) {
      ic_ev.impulses_per_unit = Param3;
    }

    ic_ev.calculated_value = supla_channel_ic_measurement::get_calculated_i(
        ic_ev.impulses_per_unit, ic_ev.counter);

    supla_channel_ic_measurement::get_cost_and_currency(
        TextParam1, Param2, ic_ev.currency, &ic_ev.total_cost,
        &ic_ev.price_per_unit,
        supla_channel_ic_measurement::get_calculated_d(ic_ev.impulses_per_unit,
                                                       ic_ev.counter));

    srpc_evtool_v1_icextended2extended(&ic_ev, &cev->value);
    return true;
  }
  return false;
}

// static
double supla_channel_ic_measurement::get_calculated_d(
    _supla_int_t impulses_per_unit, unsigned _supla_int64_t counter) {
  return supla_channel_ic_measurement::get_calculated_i(impulses_per_unit,
                                                        counter) /
         1000.00;
}

// static
_supla_int64_t supla_channel_ic_measurement::get_calculated_i(
    _supla_int_t impulses_per_unit, unsigned _supla_int64_t counter) {
  return impulses_per_unit > 0 ? counter * 1000 / impulses_per_unit : 0;
}

// static
void supla_channel_ic_measurement::get_cost_and_currency(
    char *TextParam1, int Param2, char currency[3], _supla_int_t *total_cost,
    _supla_int_t *price_per_unit, double count) {
  currency[0] = 0;
  *total_cost = 0;
  *price_per_unit = 0;

  if (TextParam1 && strlen(TextParam1) == 3) {
    memcpy(currency, TextParam1, 3);
  }

  if (Param2 > 0) {
    *price_per_unit = Param2;
    // *total_cost = (double)(Param2 * 0.0001 * count) * 100;
    *total_cost = (double)(Param2 * 0.01 * count);
  }
}

// static
void supla_channel_ic_measurement::free(void *icarr) {
  safe_array_clean(icarr, supla_channel_icarr_clean);
  safe_array_free(icarr);
}

//-----------------------------------------------------

supla_channel_thermostat_measurement::supla_channel_thermostat_measurement(
    int ChannelId, bool on, double MeasuredTemperature,
    double PresetTemperature) {
  this->MeasuredTemperature = MeasuredTemperature;
  this->PresetTemperature = PresetTemperature;
  this->on = on;
  this->ChannelId = ChannelId;
}

int supla_channel_thermostat_measurement::getChannelId(void) {
  return this->ChannelId;
}

double supla_channel_thermostat_measurement::getMeasuredTemperature(void) {
  return MeasuredTemperature;
}

double supla_channel_thermostat_measurement::getPresetTemperature(void) {
  return PresetTemperature;
}

bool supla_channel_thermostat_measurement::getOn(void) { return this->on; }

// static
void supla_channel_thermostat_measurement::free(void *tharr) {
  safe_array_clean(tharr, supla_channel_tharr_clean);
  safe_array_free(tharr);
}

//-----------------------------------------------------

supla_device_channel::supla_device_channel(int Id, int Number, int Type,
                                           int Func, int Param1, int Param2,
                                           int Param3, char *TextParam1,
                                           char *TextParam2, char *TextParam3,
                                           bool Hidden, unsigned int Flags) {
  this->Id = Id;
  this->Number = Number;
  this->Type = Type;
  this->Func = Func;
  this->Param1 = Param1;
  this->Param2 = Param2;
  this->Param3 = Param3;
  this->TextParam1 = TextParam1 ? strndup(TextParam1, 255) : NULL;
  this->TextParam2 = TextParam2 ? strndup(TextParam2, 255) : NULL;
  this->TextParam3 = TextParam3 ? strndup(TextParam3, 255) : NULL;
  this->Hidden = Hidden;
  this->Flags = Flags;
  this->Offline = Flags & SUPLA_CHANNEL_FLAG_OFFLINE_DURING_REGISTRATION;
  this->extendedValue = NULL;
  this->value_valid_to.tv_sec = 0;
  this->value_valid_to.tv_usec = 0;

  memset(this->value, 0, SUPLA_CHANNELVALUE_SIZE);
}

supla_device_channel::~supla_device_channel() {
  setExtendedValue(NULL);

  if (this->TextParam1) {
    free(this->TextParam1);
    this->TextParam1 = NULL;
  }

  if (this->TextParam2) {
    free(this->TextParam2);
    this->TextParam2 = NULL;
  }

  if (this->TextParam3) {
    free(this->TextParam3);
    this->TextParam3 = NULL;
  }
}

int supla_device_channel::getId(void) { return Id; }

int supla_device_channel::getNumber(void) { return Number; }

int supla_device_channel::getFunc(void) { return Func; }

void supla_device_channel::setFunc(int Func) { this->Func = Func; }

int supla_device_channel::getType(void) { return Type; }

int supla_device_channel::getParam1(void) { return Param1; }

bool supla_device_channel::getHidden(void) { return Hidden; }

unsigned int supla_device_channel::getFlags() { return Flags; }

bool supla_device_channel::isOffline(void) {
  if (Offline && (value_valid_to.tv_sec > 0 || value_valid_to.tv_usec)) {
    struct timeval now;
    gettimeofday(&now, NULL);

    return (now.tv_sec * 1000000 + now.tv_usec) -
                       (value_valid_to.tv_sec * 1000000 +
                        value_valid_to.tv_usec) >
                   0
               ? true
               : false;
  }

  return Offline;
}

void supla_device_channel::setOffline(bool Offline) { this->Offline = Offline; }

void supla_device_channel::getValue(char value[SUPLA_CHANNELVALUE_SIZE]) {
  memcpy(value, this->value, SUPLA_CHANNELVALUE_SIZE);
}

bool supla_device_channel::getExtendedValue(TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    return false;
  }

  if (extendedValue == NULL) {
    memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
    return false;
  }

  memcpy(ev, extendedValue, sizeof(TSuplaChannelExtendedValue));
  return true;
}

void supla_device_channel::getDouble(double *Value) {
  if (Value == NULL) return;

  switch (Type) {
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_SENSORNC:
      *Value = this->value[0] == 1 ? 1 : 0;
      break;
    case SUPLA_CHANNELTYPE_THERMOMETER:
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_DISTANCESENSOR:
    case SUPLA_CHANNELTYPE_WINDSENSOR:
    case SUPLA_CHANNELTYPE_PRESSURESENSOR:
    case SUPLA_CHANNELTYPE_RAINSENSOR:
    case SUPLA_CHANNELTYPE_WEIGHTSENSOR:
      memcpy(Value, this->value, sizeof(double));
      break;
    default:
      *Value = 0;
  }
}

void supla_device_channel::getChar(char *Value) {
  if (Value == NULL) return;
  switch (Func) {
    case SUPLA_CHANNELFNC_THERMOSTAT:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
      TThermostat_Value *tv = (TThermostat_Value *)this->value;
      *Value = tv->IsOn;
    } break;
    default:
      *Value = this->value[0];
      break;
  }
}

bool supla_device_channel::getRGBW(int *color, char *color_brightness,
                                   char *brightness, char *on_off) {
  if (color != NULL) *color = 0;

  if (color_brightness != NULL) *color_brightness = 0;

  if (brightness != NULL) *brightness = 0;

  if (on_off != NULL) *on_off = 0;

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

bool supla_device_channel::getValveValue(TValve_Value *Value) {
  if (Value == NULL) return false;
  switch (Func) {
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
      memcpy(Value, this->value, sizeof(TValve_Value));
      return true;
  }

  return false;
}

void supla_device_channel::setValue(
    const char value[SUPLA_CHANNELVALUE_SIZE],
    const unsigned _supla_int_t *validity_time_sec) {
  memcpy(this->value, value, SUPLA_CHANNELVALUE_SIZE);

  if (Type == SUPLA_CHANNELTYPE_IMPULSE_COUNTER && Param1 > 0 && Param3 > 0) {
    TDS_ImpulseCounter_Value *ic_val = (TDS_ImpulseCounter_Value *)this->value;
    ic_val->counter +=
        (unsigned _supla_int64_t)Param1 * (unsigned _supla_int64_t)Param3 / 100;

  } else if (Type == SUPLA_CHANNELTYPE_SENSORNC) {
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

  if (validity_time_sec &&
      (Flags & SUPLA_CHANNEL_FLAG_POSSIBLE_SLEEP_MODE) != 0) {
    gettimeofday(&value_valid_to, NULL);
    value_valid_to.tv_sec += (*validity_time_sec);

    database *db = new database();

    if (db->connect() == true) {
      db->update_channel_value(getId(), value, *validity_time_sec);
    }

    delete db;
  }
}

void supla_device_channel::setExtendedValue(TSuplaChannelExtendedValue *ev) {
  if (ev == NULL) {
    if (extendedValue != NULL) {
      delete extendedValue;
      extendedValue = NULL;
    }
  } else {
    if (extendedValue == NULL) {
      extendedValue = new TSuplaChannelExtendedValue;
    }
    memcpy(extendedValue, ev, sizeof(TSuplaChannelExtendedValue));
  }
}

void supla_device_channel::assignRgbwValue(char value[SUPLA_CHANNELVALUE_SIZE],
                                           int color, char color_brightness,
                                           char brightness, char on_off) {
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
    value[5] = on_off;
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
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
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
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
    case SUPLA_CHANNELFNC_VALVE_PERCENTAGE:
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
      return Param1;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      return Param1 * 100;

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

std::list<int> supla_device_channel::related_channel(void) {
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

      // Always add Param2
      result.push_back(Param2);

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
  supla_channel_temphum *result = NULL;

  if ((getType() == SUPLA_CHANNELTYPE_THERMOMETERDS18B20 ||
       getType() == SUPLA_CHANNELTYPE_THERMOMETER) &&
      getFunc() == SUPLA_CHANNELFNC_THERMOMETER) {
    result = new supla_channel_temphum(false, getId(), value);

  } else if ((getType() == SUPLA_CHANNELTYPE_DHT11 ||
              getType() == SUPLA_CHANNELTYPE_DHT22 ||
              getType() == SUPLA_CHANNELTYPE_DHT21 ||
              getType() == SUPLA_CHANNELTYPE_AM2301 ||
              getType() == SUPLA_CHANNELTYPE_AM2302 ||
              getType() == SUPLA_CHANNELTYPE_HUMIDITYSENSOR ||
              getType() == SUPLA_CHANNELTYPE_HUMIDITYANDTEMPSENSOR ||
              getType() == SUPLA_CHANNELTYPE_BRIDGE) &&
             (getFunc() == SUPLA_CHANNELFNC_THERMOMETER ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITY ||
              getFunc() == SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE)) {
    result = new supla_channel_temphum(true, getId(), value);
  }

  if (result != NULL && result->getTemperature() == -273 &&
      result->getHumidity() == -1) {
    delete result;
    result = NULL;
  }

  return result;
}

supla_channel_electricity_measurement *
supla_device_channel::getElectricityMeasurement(void) {
#ifdef SERVER_VERSION_23
  if (getType() == SUPLA_CHANNELTYPE_ELECTRICITY_METER &&
      getFunc() == SUPLA_CHANNELFNC_ELECTRICITY_METER &&
#else
  if (getFunc() == SUPLA_CHANNELFNC_ELECTRICITY_METER &&
#endif /*SERVER_VERSION_23*/
      extendedValue != NULL) {
    if (extendedValue->type == EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V1) {
      TElectricityMeter_ExtendedValue em_ev;

      if (srpc_evtool_v1_extended2emextended(extendedValue, &em_ev) == 1) {
        return new supla_channel_electricity_measurement(getId(), &em_ev,
                                                         Param2, TextParam1);
      }
    } else if (extendedValue->type ==
               EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V2) {
      TElectricityMeter_ExtendedValue_V2 em_ev;

      if (srpc_evtool_v2_extended2emextended(extendedValue, &em_ev) == 1) {
        return new supla_channel_electricity_measurement(getId(), &em_ev,
                                                         Param2, TextParam1);
      }
    }
  }

  return NULL;
}

supla_channel_ic_measurement *
supla_device_channel::getImpulseCounterMeasurement(void) {
#ifdef SERVER_VERSION_23
  if (getType() == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
#endif /*SERVER_VERSION_23*/
    switch (getFunc()) {
#ifdef SERVER_VERSION_23
      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
#endif /*SERVER_VERSION_23*/
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_HEAT_METER: {
        char value[SUPLA_CHANNELVALUE_SIZE];
        getValue(value);

        TDS_ImpulseCounter_Value *ic_val = (TDS_ImpulseCounter_Value *)value;

        return new supla_channel_ic_measurement(
            getId(), Func, ic_val, TextParam1, TextParam2, Param2, Param3);
      }
    }
#ifdef SERVER_VERSION_23
  }
#endif /*SERVER_VERSION_23*/
  return NULL;
}

supla_channel_thermostat_measurement *
supla_device_channel::getThermostatMeasurement(void) {
  switch (getType()) {
    case SUPLA_CHANNELTYPE_THERMOSTAT:
    case SUPLA_CHANNELTYPE_THERMOSTAT_HEATPOL_HOMEPLUS:
      switch (getFunc()) {
        case SUPLA_CHANNELFNC_THERMOSTAT:
        case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS: {
          char value[SUPLA_CHANNELVALUE_SIZE];
          getValue(value);
          TThermostat_Value *th_val = (TThermostat_Value *)value;

          return new supla_channel_thermostat_measurement(
              getId(), th_val->IsOn > 0, th_val->MeasuredTemperature * 0.01,
              th_val->PresetTemperature * 0.01);
        }
      }
      break;
  }

  return NULL;
}

bool supla_device_channel::converValueToExtended(void) {
  bool result = false;

#ifdef SERVER_VERSION_23
  if (getType() == SUPLA_CHANNELTYPE_IMPULSE_COUNTER) {
#endif /*SERVER_VERSION_23*/
    switch (getFunc()) {
#ifdef SERVER_VERSION_23
      case SUPLA_CHANNELFNC_ELECTRICITY_METER:
#endif /*SERVER_VERSION_23*/
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
      case SUPLA_CHANNELFNC_IC_HEAT_METER:
        char value[SUPLA_CHANNELVALUE_SIZE];
        TSuplaChannelExtendedValue ev;
        TSC_ImpulseCounter_ExtendedValue ic_ev;
        memset(&ic_ev, 0, sizeof(TSC_ImpulseCounter_ExtendedValue));

        getValue(value);

        TDS_ImpulseCounter_Value *ic_val = (TDS_ImpulseCounter_Value *)value;
        ic_ev.counter = ic_val->counter;

        srpc_evtool_v1_icextended2extended(&ic_ev, &ev);

        setExtendedValue(&ev);
        result = true;
        break;
    }
#ifdef SERVER_VERSION_23
  }
#endif /*SERVER_VERSION_23*/
  return result;
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
                                        char *TextParam1, char *TextParam2,
                                        char *TextParam3, bool Hidden,
                                        unsigned int Flags) {
  safe_array_lock(arr);

  if (find_channel(Id) == 0) {
    supla_device_channel *c = new supla_device_channel(
        Id, Number, Type, Func, Param1, Param2, Param3, TextParam1, TextParam2,
        TextParam3, Hidden, Flags);

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
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE], char *online) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      channel->getValue(value);
      if (online) {
        *online = channel->isOffline() ? 0 : 1;
      }
      result = true;
    }

    safe_array_unlock(arr);
  }

  return result;
}

bool supla_device_channels::get_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    if (channel) {
      result = channel->getExtendedValue(value);
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

bool supla_device_channels::get_channel_valve_value(int ChannelID,
                                                    TValve_Value *Value) {
  bool result = false;

  if (ChannelID) {
    safe_array_lock(arr);
    supla_device_channel *channel = find_channel(ChannelID);

    result = channel && channel->getValveValue(Value);

    safe_array_unlock(arr);
  }

  return result;
}

void supla_device_channels::set_channel_value(
    int ChannelID, char value[SUPLA_CHANNELVALUE_SIZE],
    bool *converted2extended, const unsigned _supla_int_t *validity_time_sec) {
  if (ChannelID == 0) return;

  if (converted2extended) {
    *converted2extended = false;
  }

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    channel->setValue(value, validity_time_sec);

    if (channel->converValueToExtended()) {
      if (converted2extended) {
        *converted2extended = true;
      }
    }
  }

  safe_array_unlock(arr);
}

void supla_device_channels::set_channel_offline(int ChannelID, bool Offline) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    channel->setOffline(Offline);
  }

  safe_array_unlock(arr);
}

void supla_device_channels::set_channel_extendedvalue(
    int ChannelID, TSuplaChannelExtendedValue *ev) {
  if (ChannelID == 0) return;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) channel->setExtendedValue(ev);

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

int supla_device_channels::get_channel_type(int ChannelID) {
  if (ChannelID == 0) return 0;

  int Type = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) Type = channel->getType();

  safe_array_unlock(arr);

  return Type;
}

std::list<int> supla_device_channels::mr_channel(int ChannelID, bool Master) {
  std::list<int> result;
  if (ChannelID == 0) return result;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel)
    result = Master ? channel->master_channel() : channel->related_channel();

  safe_array_unlock(arr);

  return result;
}

std::list<int> supla_device_channels::master_channel(int ChannelID) {
  return mr_channel(ChannelID, true);
}

std::list<int> supla_device_channels::related_channel(int ChannelID) {
  return mr_channel(ChannelID, false);
}

bool supla_device_channels::channel_exists(int ChannelID) {
  bool result = false;

  safe_array_lock(arr);

  if (find_channel(ChannelID) != NULL) result = true;

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::set_channels_value(
    TDS_SuplaDeviceChannel_B *schannel_b, TDS_SuplaDeviceChannel_C *schannel_c,
    int count) {
  if (schannel_b != NULL) {
    for (int a = 0; a < count; a++)
      set_channel_value(get_channel_id(schannel_b[a].Number),
                        schannel_b[a].value, NULL, 0);
  } else {
    for (int a = 0; a < count; a++)
      set_channel_value(get_channel_id(schannel_c[a].Number),
                        schannel_c[a].value, NULL, 0);
  }
}

void supla_device_channels::on_device_registered(
    supla_user *user, int DeviceId, TDS_SuplaDeviceChannel_B *schannel_b,
    TDS_SuplaDeviceChannel_C *schannel_c, int count) {
  int ChannelId = 0;

  for (int a = 0; a < count; a++) {
    if (schannel_b != NULL) {
      ChannelId = get_channel_id(schannel_b[a].Number);
    } else {
      ChannelId = get_channel_id(schannel_c[a].Number);
    }

    if (ChannelId > 0) {
      user->on_channel_become_online(DeviceId, ChannelId);
    }
  }
}

int supla_device_channels::get_channel_id(unsigned char ChannelNumber) {
  int result = 0;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel_by_number(ChannelNumber);

  if (channel) result = channel->getId();

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::async_set_channel_value(
    void *srpc, supla_device_channel *channel, int SenderID,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  TSD_SuplaChannelNewValue s;
  memset(&s, 0, sizeof(TSD_SuplaChannelNewValue));

  s.ChannelNumber = channel->getNumber();
  s.DurationMS = channel->getValueDuration();
  s.SenderID = SenderID;
  memcpy(s.value, value, SUPLA_CHANNELVALUE_SIZE);

  srpc_sd_async_set_channel_value(srpc, &s);
}

void supla_device_channels::set_device_channel_value(
    void *srpc, int SenderID, int ChannelID,
    const char value[SUPLA_CHANNELVALUE_SIZE]) {
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isValueWritable()) {
    async_set_channel_value(srpc, channel, SenderID, value);
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

  if (channel) {
    if (channel->isCharValueWritable()) {
      char v[SUPLA_CHANNELVALUE_SIZE];
      memset(v, 0, SUPLA_CHANNELVALUE_SIZE);
      channel->assignCharValue(v, value);

      async_set_channel_value(srpc, channel, SenderID, v);

      result = true;
    } else if (channel->isRgbwValueWritable()) {
      int color = 0;
      char color_brightness = 0;
      char brightness = 0;
      char on_off = 0;

      if (channel->getRGBW(&color, &color_brightness, &brightness, &on_off)) {
        if (value > 0) {
          color_brightness = 100;
          brightness = 100;
        } else {
          color_brightness = 0;
          brightness = 0;
        }

        on_off = RGBW_BRIGHTNESS_ONOFF | RGBW_COLOR_ONOFF;

        result =
            set_device_channel_rgbw_value(srpc, SenderID, ChannelID, color,
                                          color_brightness, brightness, on_off);
      }
    }
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::set_device_channel_rgbw_value(
    void *srpc, int SenderID, int ChannelID, int color, char color_brightness,
    char brightness, char on_off) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel && channel->isRgbwValueWritable()) {
    char v[SUPLA_CHANNELVALUE_SIZE];
    memset(v, 0, SUPLA_CHANNELVALUE_SIZE);

    channel->assignRgbwValue(v, color, color_brightness, brightness, on_off);

    async_set_channel_value(srpc, channel, SenderID, v);

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

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_temphum *temphum = channel->getTempHum();

      if (temphum != NULL) safe_array_add(tarr, temphum);
    }
  }

  safe_array_unlock(arr);
}

bool supla_device_channels::get_channel_rgbw_value(int ChannelID, int *color,
                                                   char *color_brightness,
                                                   char *brightness,
                                                   char *on_off) {
  bool result = false;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    int _color = 0;
    char _color_brightness = 0;
    char _brightness = 0;
    char _on_off = 0;

    result =
        channel->getRGBW(&_color, &_color_brightness, &_brightness, &_on_off);

    if (result == true) {
      if (color != NULL) *color = _color;

      if (color_brightness) *color_brightness = _color_brightness;

      if (brightness != NULL) *brightness = _brightness;

      if (on_off != NULL) *on_off = _on_off;
    }
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_electricity_measurements(void *emarr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_electricity_measurement *em =
          channel->getElectricityMeasurement();
      if (em) {
        safe_array_add(emarr, em);
      }
    }
  }

  safe_array_unlock(arr);
}

supla_channel_electricity_measurement *
supla_device_channels::get_electricity_measurement(int ChannelID) {
  supla_channel_electricity_measurement *result = NULL;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    result = channel->getElectricityMeasurement();
  }

  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::get_ic_measurements(void *icarr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_ic_measurement *ic =
          channel->getImpulseCounterMeasurement();
      if (ic) {
        safe_array_add(icarr, ic);
      }
    }
  }

  safe_array_unlock(arr);
}

void supla_device_channels::get_thermostat_measurements(void *tharr) {
  int a;
  safe_array_lock(arr);

  for (a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        (supla_device_channel *)safe_array_get(arr, a);

    if (channel != NULL && !channel->isOffline()) {
      supla_channel_thermostat_measurement *th =
          channel->getThermostatMeasurement();
      if (th) {
        safe_array_add(tharr, th);
      }
    }
  }

  safe_array_unlock(arr);
}

supla_channel_ic_measurement *supla_device_channels::get_ic_measurement(
    int ChannelID) {
  supla_channel_ic_measurement *result = NULL;

  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel != NULL) {
    result = channel->getImpulseCounterMeasurement();
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::calcfg_request(void *srpc, int SenderID,
                                           int ChannelID,
                                           bool SuperUserAuthorized,
                                           TCS_DeviceCalCfgRequest_B *request) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    TSD_DeviceCalCfgRequest drequest;
    memset(&drequest, 0, sizeof(TSD_DeviceCalCfgRequest));

    drequest.SenderID = SenderID;
    drequest.ChannelNumber = channel->getNumber();
    drequest.Command = request->Command;
    drequest.SuperUserAuthorized = SuperUserAuthorized;
    drequest.DataType = request->DataType;
    drequest.DataSize = request->DataSize > SUPLA_CALCFG_DATA_MAXSIZE
                            ? SUPLA_CALCFG_DATA_MAXSIZE
                            : request->DataSize;
    memcpy(drequest.Data, request->Data, SUPLA_CALCFG_DATA_MAXSIZE);

    srpc_sd_async_device_calcfg_request(srpc, &drequest);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::get_channel_state(
    void *srpc, int SenderID, TCSD_ChannelStateRequest *request) {
  if (request == NULL) {
    return false;
  }

  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(request->ChannelID);

  if (channel) {
    TCSD_ChannelStateRequest drequest;
    memcpy(&drequest, request, sizeof(TCSD_ChannelStateRequest));

    drequest.SenderID = SenderID;
    drequest.ChannelNumber = channel->getNumber();

    srpc_csd_async_get_channel_state(srpc, &drequest);
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

bool supla_device_channels::get_channel_complex_value(
    channel_complex_value *value, int ChannelID) {
  bool result = false;
  safe_array_lock(arr);

  supla_device_channel *channel = find_channel(ChannelID);

  if (channel) {
    value->online = true;
    value->hidden_channel = channel->getHidden();
    value->function = channel->getFunc();

    switch (value->function) {
      case SUPLA_CHANNELFNC_THERMOMETER:
      case SUPLA_CHANNELFNC_HUMIDITY:
      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
        supla_channel_temphum *tempHum = channel->getTempHum();
        if (tempHum) {
          value->temperature = tempHum->getTemperature();
          value->humidity = tempHum->getHumidity();
          delete tempHum;
        }
      } break;

      case SUPLA_CHANNELFNC_WINDSENSOR:
        channel->getDouble(&value->wind);
        break;
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
        channel->getDouble(&value->pressure);
        break;
      case SUPLA_CHANNELFNC_RAINSENSOR:
        channel->getDouble(&value->rain);
        break;
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        channel->getDouble(&value->weight);
        break;
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      case SUPLA_CHANNELFNC_MAILSENSOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_VALVE_OPENCLOSE:
      case SUPLA_CHANNELFNC_VALVE_PERCENTAGE: {
        char cv[SUPLA_CHANNELVALUE_SIZE];
        channel->getChar(cv);
        value->hi = cv[0] > 0;
      } break;

      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        channel->getRGBW(&value->color, &value->color_brightness,
                         &value->brightness, &value->on_off);
        break;

      case SUPLA_CHANNELFNC_DEPTHSENSOR:
        channel->getDouble(&value->depth);
        break;
      case SUPLA_CHANNELFNC_DISTANCESENSOR:
        channel->getDouble(&value->distance);
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
        char cv[SUPLA_CHANNELVALUE_SIZE];
        channel->getChar(cv);
        value->shut = cv[0];
        break;
    }
    result = true;
  }

  safe_array_unlock(arr);

  return result;
}

std::list<int> supla_device_channels::get_channel_ids(void) {
  std::list<int> result;
  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        static_cast<supla_device_channel *>(safe_array_get(arr, a));
    if (channel) {
      result.push_back(channel->getId());
    }
  }
  safe_array_unlock(arr);

  return result;
}

void supla_device_channels::set_channel_function(int ChannelId, int Func) {
  supla_device_channel *channel = find_channel(ChannelId);

  if (channel) {
    channel->setFunc(Func);
  }
}

void supla_device_channels::get_functions_request(void *srpc) {
  TSD_ChannelFunctions result;
  memset(&result, 0, sizeof(TSD_ChannelFunctions));

  safe_array_lock(arr);
  for (int a = 0; a < safe_array_count(arr); a++) {
    supla_device_channel *channel =
        static_cast<supla_device_channel *>(safe_array_get(arr, a));
    if (channel && channel->getNumber() >= 0 &&
        channel->getNumber() < SUPLA_CHANNELMAXCOUNT) {
      if (result.ChannelCount >= channel->getNumber()) {
        result.ChannelCount = channel->getNumber() + 1;
      }

      result.Functions[channel->getNumber()] = channel->getFunc();
    }
  }
  safe_array_unlock(arr);

  srpc_sd_async_get_channel_functions_result(srpc, &result);
}
