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

#include "impulse_counter_config.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "srpc/srpc.h"

#define INITIAL_VALUE_MIN -100000000
#define INITIAL_VALUE_MAX 100000000
#define IMPULSES_PER_UNIT_MIN 0
#define IMPULSES_PER_UNIT_MAX 1000000

#define FIELD_PRICE_PER_UNIT 1
#define FIELD_CURRENCY 2
#define FIELD_CUSTOM_UNIT 3
#define FIELD_IMPULSES_PER_UNIT 4
#define FIELD_INITIAL_VALUE 5
#define FIELD_ADD_TO_HISTORY 6

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> impulse_counter_config::field_map = {
    {FIELD_PRICE_PER_UNIT, "pricePerUnit"},
    {FIELD_CURRENCY, "currency"},
    {FIELD_CUSTOM_UNIT, "unit"},
    {FIELD_IMPULSES_PER_UNIT, "impulsesPerUnit"},
    {FIELD_INITIAL_VALUE, "initialValue"},
    {FIELD_ADD_TO_HISTORY, "addToHistory"}};

impulse_counter_config::impulse_counter_config(void) : supla_json_config() {}

impulse_counter_config::impulse_counter_config(supla_json_config *root)
    : supla_json_config(root) {}

bool impulse_counter_config::should_be_added_to_history(void) {
  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *item =
      cJSON_GetObjectItem(root, field_map.at(FIELD_ADD_TO_HISTORY).c_str());
  return item && cJSON_IsBool(item) && cJSON_IsTrue(item);
}

double impulse_counter_config::get_double_value(const char *key, double min,
                                                double max, double multiplier) {
  cJSON *root = get_user_root();
  if (!root) {
    return 0;
  }

  cJSON *json_value = cJSON_GetObjectItem(root, key);
  if (json_value && cJSON_IsNumber(json_value)) {
    double value = json_value->valuedouble * multiplier;
    if (value > max) {
      return max;
    } else if (value < min) {
      return min;
    }
    return value;
  }

  return 0;
}

double impulse_counter_config::get_initial_value(void) {
  return get_double_value(field_map.at(FIELD_INITIAL_VALUE).c_str(),
                          INITIAL_VALUE_MIN, INITIAL_VALUE_MAX, 0.001);
}

unsigned int impulse_counter_config::get_impulses_per_unit(void) {
  return get_double_value(field_map.at(FIELD_IMPULSES_PER_UNIT).c_str(),
                          IMPULSES_PER_UNIT_MIN, IMPULSES_PER_UNIT_MAX, 1.0);
}

void impulse_counter_config::add_initial_value(
    unsigned _supla_int64_t *counter) {
  if (!counter) {
    return;
  }

  unsigned _supla_int64_t left = ULONG_MAX - (*counter);
  double initial_value = get_initial_value();
  int impulses_per_unit = get_impulses_per_unit();

  if (initial_value == 0 || impulses_per_unit == 0) {
    return;
  }

  bool minus = false;
  if (initial_value < 0) {
    minus = true;
    initial_value *= -1;
  }

  unsigned _supla_int64_t impulses_added = initial_value * impulses_per_unit;

  if (minus) {
    if (impulses_added > (*counter)) {
      impulses_added = (*counter);
    }
    *counter -= impulses_added;
  } else {
    if (impulses_added > left) {
      impulses_added = left;
    }
    *counter += impulses_added;
  }
}

bool impulse_counter_config::get_custom_unit(cJSON *user_root,
                                             char custom_unit[9],
                                             int use_defaults_for_function) {
  *custom_unit = {};

  if (!user_root) {
    return false;
  }

  string unit;

  if (get_string(user_root, field_map.at(FIELD_CUSTOM_UNIT).c_str(), &unit) &&
      unit.size() && unit.size() < 9) {
    snprintf(custom_unit, 9, "%s", unit.c_str());  // NOLINT
    return true;
  } else {
    switch (use_defaults_for_function) {
      case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        snprintf(custom_unit, 9, "kWh");  // NOLINT
        return true;
      case SUPLA_CHANNELFNC_IC_GAS_METER:
      case SUPLA_CHANNELFNC_IC_WATER_METER:
        // UTF(³) == 0xc2b3
        snprintf(custom_unit, 9, "m%c%c", 0xc2, 0xb3);  // NOLINT
        return true;
      case SUPLA_CHANNELFNC_IC_HEAT_METER:
        snprintf(custom_unit, 9, "GJ");  // NOLINT
        return true;
    }
  }

  return false;
}

void impulse_counter_config::get_custom_unit(char custom_unit[9],
                                             int use_defaults_for_function) {
  get_custom_unit(get_user_root(), custom_unit, use_defaults_for_function);
}

int impulse_counter_config::get_price_per_unit(void) {
  cJSON *user_root = get_user_root();
  if (!user_root) {
    return 0;
  }

  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_PRICE_PER_UNIT).c_str(),
                 &dbl_value)) {
    return dbl_value;
  }

  return 0;
}

bool impulse_counter_config::get_currency(cJSON *user_root, string *currency) {
  if (!user_root || !currency) {
    return false;
  }

  if (get_string(user_root, field_map.at(FIELD_CURRENCY).c_str(), currency) &&
      currency->size() == 3) {
    return true;
  }

  *currency = "";
  return false;
}

std::string impulse_counter_config::get_currency(void) {
  string currency;
  get_currency(get_user_root(), &currency);

  return currency;
}

void impulse_counter_config::merge(supla_json_config *_dst) {
  impulse_counter_config dst(_dst);
  supla_json_config::merge(get_user_root(), dst.get_user_root(), field_map,
                           false);
}

void impulse_counter_config::set_config(TChannelConfig_ImpulseCounter *config) {
  if (!config) {
    return;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return;
  }

  set_item_value(user_root, field_map.at(FIELD_PRICE_PER_UNIT).c_str(),
                 cJSON_Number, true, nullptr, nullptr, config->PricePerUnit);

  if (config->Currency[0] && config->Currency[1] && config->Currency[2]) {
    string currency;
    currency.append(config->Currency, 3);
    set_item_value(user_root, field_map.at(FIELD_CURRENCY).c_str(),
                   cJSON_String, true, nullptr, currency.c_str(), 0);
  }

  if (strnlen(config->CustomUnit, 9) <= 8) {
    set_item_value(user_root, field_map.at(FIELD_CUSTOM_UNIT).c_str(),
                   cJSON_String, true, nullptr, config->CustomUnit, 0);
  }

  if (config->ImpulsesPerUnit >= IMPULSES_PER_UNIT_MIN &&
      config->ImpulsesPerUnit <= IMPULSES_PER_UNIT_MAX) {
    set_item_value(user_root, field_map.at(FIELD_IMPULSES_PER_UNIT).c_str(),
                   cJSON_Number, true, nullptr, nullptr,
                   config->ImpulsesPerUnit);
  }

  if (config->InitialValue >= INITIAL_VALUE_MIN &&
      config->InitialValue <= INITIAL_VALUE_MAX) {
    set_item_value(user_root, field_map.at(FIELD_INITIAL_VALUE).c_str(),
                   cJSON_Number, true, nullptr, nullptr, config->InitialValue);
  }

  set_item_value(user_root, field_map.at(FIELD_ADD_TO_HISTORY).c_str(),
                 config->AddToHistory ? cJSON_True : cJSON_False, true, nullptr,
                 nullptr, 0);
}

bool impulse_counter_config::get_config(TChannelConfig_ImpulseCounter *config) {
  if (!config) {
    return false;
  }

  cJSON *user_root = get_user_root();
  if (!user_root) {
    return false;
  }

  bool result = false;
  double dbl_value = 0;

  if (get_double(user_root, field_map.at(FIELD_PRICE_PER_UNIT).c_str(),
                 &dbl_value)) {
    config->PricePerUnit = dbl_value;
    result = true;
  }

  string str_value;
  if (get_currency(user_root, &str_value)) {
    memcpy(config->Currency, str_value.c_str(), 3);
    result = true;
  }

  if (get_custom_unit(user_root, config->CustomUnit, 0)) {
    result = true;
  }

  if (cJSON_GetObjectItem(user_root,
                          field_map.at(FIELD_IMPULSES_PER_UNIT).c_str())) {
    config->ImpulsesPerUnit = get_impulses_per_unit();
    result = true;
  }

  if (get_double(user_root, field_map.at(FIELD_INITIAL_VALUE).c_str(),
                 &dbl_value)) {
    config->InitialValue = dbl_value;
    result = true;
  }

  bool bool_value = false;
  if (get_bool(user_root, field_map.at(FIELD_ADD_TO_HISTORY).c_str(),
               &bool_value)) {
    config->AddToHistory = bool_value ? 1 : 0;
    result = true;
  }

  return result;
}
