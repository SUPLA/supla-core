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

#include "device_json_config.h"

#include "log.h"

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_config::field_map = {
    {SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED, "statusLed"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS, "screenBrightness"},
    {SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME, "buttonVolume"},
    {SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE, "userInterface"},
    {SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC, "automaticTimeSync"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY, "offDelay"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT, "content"},
    {SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE, "offDelayType"},
    {SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED, "powerStatusLed"}};

const map<unsigned _supla_int16_t, string>
    device_json_config::home_screen_content_map = {
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE, "NONE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE, "TEMPERATURE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_AND_HUMIDITY,
         "TEMPERATURE_AND_HUMIDITY"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME, "TIME"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TIME_DATE, "TIME_DATE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_TEMPERATURE_TIME, "TEMPERATURE_TIME"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MAIN_AND_AUX_TEMPERATURE,
         "MAIN_AND_AUX_TEMPERATURE"},
        {SUPLA_DEVCFG_HOME_SCREEN_CONTENT_MODE_OR_TEMPERATURE,
         "MODE_OR_TEMPERATURE"}};

const char device_json_config::content_available[] =
    "homeScreenContentAvailable";

const char device_json_config::disabled_str[] = "disabled";

const char device_json_config::min_allowed_temperature[] =
    "minAllowedTemperatureSetpointFromLocalUI";

const char device_json_config::max_allowed_temperature[] =
    "maxAllowedTemperatureSetpointFromLocalUI";

const char device_json_config::level_str[] = "level";
const char device_json_config::adjustment_str[] = "adjustment";

device_json_config::device_json_config(void) : supla_json_config() {}

device_json_config::device_json_config(supla_json_config *root)
    : supla_json_config(root) {}

device_json_config::~device_json_config(void) {}

string device_json_config::status_led_to_string(unsigned char status) {
  switch (status) {
    case SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED:
      return "OFF_WHEN_CONNECTED";
    case SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF:
      return "ALWAYS_OFF";
  }

  return "ON_WHEN_CONNECTED";
}

unsigned char device_json_config::string_to_status_led(
    const std::string &status) {
  if (status == "OFF_WHEN_CONNECTED") {
    return SUPLA_DEVCFG_STATUS_LED_OFF_WHEN_CONNECTED;
  } else if (status == "ALWAYS_OFF") {
    return SUPLA_DEVCFG_STATUS_LED_ALWAYS_OFF;
  }

  return SUPLA_DEVCFG_STATUS_LED_ON_WHEN_CONNECTED;
}

string device_json_config::power_status_led_to_string(unsigned char status) {
  return status ? "DISABLED" : "ENABLED";
}

unsigned char device_json_config::string_to_power_status_led(
    const std::string &status) {
  return status == "DISABLED" ? 1 : 0;
}

string device_json_config::home_screen_content_to_string(
    unsigned char content) {
  for (auto it = home_screen_content_map.cbegin();
       it != home_screen_content_map.cend(); ++it) {
    if (it->first == content) {
      return it->second;
    }
  }

  return "";
}

unsigned char device_json_config::string_to_home_screen_content(
    const std::string &content) {
  for (auto it = home_screen_content_map.cbegin();
       it != home_screen_content_map.cend(); ++it) {
    if (it->second == content) {
      return it->first;
    }
  }

  return SUPLA_DEVCFG_HOME_SCREEN_CONTENT_NONE;
}

void device_json_config::set_status_led(TDeviceConfig_StatusLed *status_led) {
  if (status_led && status_led->StatusLedType >= 0 &&
      status_led->StatusLedType <= 2) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED),
                   cJSON_String, true, nullptr,
                   status_led_to_string(status_led->StatusLedType).c_str(), 0);
  }
}

void device_json_config::set_power_status_led(
    TDeviceConfig_PowerStatusLed *status_led) {
  if (status_led && status_led->Disabled >= 0 && status_led->Disabled <= 1) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED),
                   cJSON_String, true, nullptr,
                   power_status_led_to_string(status_led->Disabled).c_str(), 0);
  }
}

void device_json_config::set_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (brightness) {
    cJSON *br = cJSON_CreateObject();
    if (br) {
      if (brightness->Automatic == 1) {
        set_item_value(br, level_str, cJSON_String, true, nullptr, "auto", 0);
        set_item_value(br, adjustment_str, cJSON_Number, true, nullptr, nullptr,
                       brightness->AdjustmentForAutomatic);
      } else if (brightness->ScreenBrightness >= 0 &&
                 brightness->ScreenBrightness <= 100) {
        set_item_value(br, level_str, cJSON_Number, true, nullptr, nullptr,
                       brightness->ScreenBrightness);
      }

      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_Object, true, br, nullptr, 0);
    }
  }
}

void device_json_config::set_button_volume(TDeviceConfig_ButtonVolume *volume) {
  unsigned char v = volume ? volume->Volume : 0;

  if (v <= 100) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME),
                   cJSON_Number, true, nullptr, nullptr, v);
  }
}

void device_json_config::set_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  if (disabled && disabled->DisableUserInterface >= 0 &&
      disabled->DisableUserInterface <= 2) {
    cJSON *ui = cJSON_CreateObject();
    if (ui) {
      if (disabled->DisableUserInterface == 2) {
        cJSON_AddStringToObject(ui, disabled_str, "partial");
        cJSON_AddNumberToObject(
            ui, min_allowed_temperature,
            disabled->minAllowedTemperatureSetpointFromLocalUI);
        cJSON_AddNumberToObject(
            ui, max_allowed_temperature,
            disabled->maxAllowedTemperatureSetpointFromLocalUI);
      } else {
        cJSON_AddBoolToObject(ui, disabled_str,
                              disabled->DisableUserInterface ? true : false);
      }

      set_item_value(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE),
          cJSON_Object, true, ui, nullptr, 0);
    }
  }
}

void device_json_config::set_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  if (time_sync && (time_sync->AutomaticTimeSync == 0 ||
                    time_sync->AutomaticTimeSync == 1)) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC),
                   time_sync->AutomaticTimeSync ? cJSON_True : cJSON_False,
                   true, nullptr, nullptr, 0);
  }
}

cJSON *device_json_config::get_root(bool create,
                                    unsigned _supla_int64_t field) {
  cJSON *root = get_user_root();

  if (field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY &&
      field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT &&
      field != SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE) {
    return root;
  }

  if (root) {
    const char name[] = "homeScreen";
    cJSON *result = cJSON_GetObjectItem(root, name);
    if (!result && create) {
      result = cJSON_AddObjectToObject(root, name);
    }

    return result;
  }
  return nullptr;
}

void device_json_config::set_home_screen_off_delay(
    TDeviceConfig_HomeScreenOffDelay *delay) {
  if (delay) {
    set_item_value(
        get_root(true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
        cJSON_Number, true, nullptr, nullptr, delay->HomeScreenOffDelayS);
  }
}

void device_json_config::set_home_screen_off_delay_type(
    TDeviceConfig_HomeScreenOffDelayType *type) {
  if (type) {
    set_item_value(
        get_root(true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
        cJSON_String, true, nullptr,
        type->HomeScreenOffDelayType ==
                SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ENABLED_WHEN_DARK
            ? "ENABLED_WHEN_DARK"
            : "ALWAYS_ENABLED",
        0);
  }
}

void device_json_config::set_home_screen_content(
    TDeviceConfig_HomeScreenContent *content) {
  cJSON *root = get_root(true, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT);
  string content_str =
      home_screen_content_to_string(content->HomeScreenContent);
  if (!content_str.empty()) {
    set_item_value(root,
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT),
                   cJSON_String, true, nullptr, content_str.c_str(), 0);
  }

  cJSON *content_items =
      cJSON_GetObjectItem(get_properties_root(), content_available);

  if (content_items) {
    cJSON_DetachItemViaPointer(get_properties_root(), content_items);
    cJSON_Delete(content_items);
  }

  content_items =
      cJSON_AddArrayToObject(get_properties_root(), content_available);
  if (content_items) {
    unsigned char size = sizeof(content->ContentAvailable) * 8;
    unsigned _supla_int64_t n = 1;
    for (unsigned char a = 0; a < size; a++) {
      if (content->ContentAvailable & n) {
        content_str = home_screen_content_to_string(n);
        if (!content_str.empty()) {
          cJSON *mode_json = cJSON_CreateString(content_str.c_str());
          if (mode_json) {
            cJSON_AddItemToArray(content_items, mode_json);
          }
        }
      }
      n <<= 1;
    }
  }
}

void device_json_config::set_config(TSDS_SetDeviceConfig *config) {
  if (!config) {
    return;
  }

  unsigned short offset = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (config->Fields & it->first) {
      unsigned _supla_int16_t left = config->ConfigSize;
      if (left > SUPLA_DEVICE_CONFIG_MAXSIZE) {
        left = SUPLA_DEVICE_CONFIG_MAXSIZE;
      }
      if (left > offset) {
        left -= offset;
      } else {
        return;
      }

      void *ptr = &config->Config[offset];
      unsigned short size = 0;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_StatusLed))) {
            set_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          if (left >= (size = sizeof(TDeviceConfig_ScreenBrightness))) {
            set_screen_brightness(
                static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          if (left >= (size = sizeof(TDeviceConfig_ButtonVolume))) {
            set_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          if (left >= (size = sizeof(TDeviceConfig_DisableUserInterface))) {
            set_user_interface_disabled(
                static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          if (left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync))) {
            set_automatic_time_sync(
                static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelay))) {
            set_home_screen_off_delay(
                static_cast<TDeviceConfig_HomeScreenOffDelay *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenContent))) {
            set_home_screen_content(
                static_cast<TDeviceConfig_HomeScreenContent *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE:
          if (left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelayType))) {
            set_home_screen_off_delay_type(
                static_cast<TDeviceConfig_HomeScreenOffDelayType *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED:
          if (left >= (size = sizeof(TDeviceConfig_PowerStatusLed))) {
            set_power_status_led(
                static_cast<TDeviceConfig_PowerStatusLed *>(ptr));
          }
          break;
      }

      offset += size;
    }
  }
}

bool device_json_config::get_status_led(TDeviceConfig_StatusLed *status_led) {
  std::string str_value;
  if (status_led &&
      get_string(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED).c_str(),
                 &str_value)) {
    status_led->StatusLedType = string_to_status_led(str_value);
    return true;
  }
  return false;
}

bool device_json_config::get_power_status_led(
    TDeviceConfig_PowerStatusLed *status_led) {
  std::string str_value;
  if (status_led &&
      get_string(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED).c_str(),
          &str_value)) {
    status_led->Disabled = string_to_power_status_led(str_value);
    return true;
  }
  return false;
}

bool device_json_config::get_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (!brightness) {
    return false;
  }

  cJSON *parent = cJSON_GetObjectItem(
      get_user_root(),
      field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str());

  if (parent) {
    double value = 0;
    if (get_double(parent, level_str, &value) && value >= 0 && value <= 100) {
      brightness->Automatic = 0;
      brightness->ScreenBrightness = value;
      return true;
    }

    std::string str_value;
    if (get_string(parent, level_str, &str_value) && str_value == "auto") {
      brightness->Automatic = 1;
      brightness->ScreenBrightness = 0;

      if (get_double(parent, adjustment_str, &value)) {
        brightness->AdjustmentForAutomatic = value;
      }

      return true;
    }
  }

  return false;
}

bool device_json_config::get_button_volume(TDeviceConfig_ButtonVolume *volume) {
  double value = 0;
  if (volume &&
      get_double(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME).c_str(),
                 &value) &&
      value >= 0 && value <= 100) {
    volume->Volume = value;
    return true;
  }

  return false;
}

bool device_json_config::get_user_interface_disabled(
    TDeviceConfig_DisableUserInterface *disabled) {
  bool bool_value = 0;
  double dbl_min = 0;
  double dbl_max = 0;
  string str_value;

  if (disabled) {
    cJSON *parent = cJSON_GetObjectItem(
        get_user_root(),
        field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE).c_str());

    disabled->minAllowedTemperatureSetpointFromLocalUI = 0;
    disabled->maxAllowedTemperatureSetpointFromLocalUI = 0;

    if (get_bool(parent, disabled_str, &bool_value)) {
      disabled->DisableUserInterface = bool_value ? 1 : 0;
      return true;
    } else if (get_string(parent, disabled_str, &str_value) &&
               str_value == "partial" &&
               get_double(parent, min_allowed_temperature, &dbl_min) &&
               get_double(parent, max_allowed_temperature, &dbl_max)) {
      disabled->DisableUserInterface = 2;
      disabled->minAllowedTemperatureSetpointFromLocalUI = dbl_min;
      disabled->maxAllowedTemperatureSetpointFromLocalUI = dbl_max;
      return true;
    }
  }

  return false;
}

bool device_json_config::get_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  bool value = 0;
  if (time_sync &&
      get_bool(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC).c_str(),
          &value)) {
    time_sync->AutomaticTimeSync = value ? 1 : 0;
    return true;
  }

  return false;
}

bool device_json_config::get_home_screen_off_delay(
    TDeviceConfig_HomeScreenOffDelay *delay) {
  double value = 0;
  if (delay &&
      get_double(
          get_root(false, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY).c_str(),
          &value)) {
    delay->HomeScreenOffDelayS = value;
    return true;
  }

  return false;
}

bool device_json_config::get_home_screen_off_delay_type(
    TDeviceConfig_HomeScreenOffDelayType *type) {
  if (type) {
    string str_value;

    if (get_string(
            get_root(false,
                     SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE),
            field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE)
                .c_str(),
            &str_value)) {
      type->HomeScreenOffDelayType =
          str_value == "ENABLED_WHEN_DARK"
              ? SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ENABLED_WHEN_DARK
              : SUPLA_DEVCFG_HOME_SCREEN_OFF_DELAY_TYPE_ALWAYS_ENABLED;

      return true;
    }
  }

  return false;
}

bool device_json_config::get_home_screen_content(
    TDeviceConfig_HomeScreenContent *content) {
  std::string str_value;
  bool result = false;

  if (content) {
    cJSON *root =
        get_root(false, SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY);
    if (get_string(
            root,
            field_map.at(SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT).c_str(),
            &str_value)) {
      content->HomeScreenContent = string_to_home_screen_content(str_value);
      result = true;
    }

    content->ContentAvailable = 0;

    cJSON *modes =
        cJSON_GetObjectItem(get_properties_root(), content_available);

    if (modes && cJSON_IsArray(modes)) {
      result = true;
      for (int a = 0; a < cJSON_GetArraySize(modes); a++) {
        cJSON *item = cJSON_GetArrayItem(modes, a);
        if (item && cJSON_IsString(item)) {
          content->ContentAvailable |=
              string_to_home_screen_content(cJSON_GetStringValue(item));
        }
      }
    }
  }
  return result;
}

unsigned _supla_int64_t device_json_config::get_available_fields(void) {
  unsigned _supla_int64_t result = 0;
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, it->first);
    if (root && cJSON_GetObjectItem(root, it->second.c_str())) {
      result |= it->first;
    }
  }
  return result;
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t fields,
                                    unsigned _supla_int64_t *fields_left) {
  if (fields_left) {
    *fields_left = 0;
    // map is sorted
    for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
      if (fields & it->first) {
        *fields_left |= it->first;
      }
    }
  }

  if (!config) {
    return;
  }

  config->Fields = 0;
  config->ConfigSize = 0;

  // map is sorted
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      void *ptr = &config->Config[config->ConfigSize];
      unsigned short size = 0;
      bool field_set = false;

      unsigned _supla_int16_t left =
          SUPLA_DEVICE_CONFIG_MAXSIZE - config->ConfigSize;

      switch (it->first) {
        case SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED:
          field_set =
              left >= (size = sizeof(TDeviceConfig_StatusLed)) &&
              get_status_led(static_cast<TDeviceConfig_StatusLed *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreenBrightness)) &&
                      get_screen_brightness(
                          static_cast<TDeviceConfig_ScreenBrightness *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME:
          field_set =
              left >= (size = sizeof(TDeviceConfig_ButtonVolume)) &&
              get_button_volume(static_cast<TDeviceConfig_ButtonVolume *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_USER_INTERFACE:
          field_set =
              left >= (size = sizeof(TDeviceConfig_DisableUserInterface)) &&
              get_user_interface_disabled(
                  static_cast<TDeviceConfig_DisableUserInterface *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          field_set =
              left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync)) &&
              get_automatic_time_sync(
                  static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelay)) &&
              get_home_screen_off_delay(
                  static_cast<TDeviceConfig_HomeScreenOffDelay *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenContent)) &&
              get_home_screen_content(
                  static_cast<TDeviceConfig_HomeScreenContent *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_OFF_DELAY_TYPE:
          field_set =
              left >= (size = sizeof(TDeviceConfig_HomeScreenOffDelayType)) &&
              get_home_screen_off_delay_type(
                  static_cast<TDeviceConfig_HomeScreenOffDelayType *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_POWER_STATUS_LED:
          field_set = left >= (size = sizeof(TDeviceConfig_PowerStatusLed)) &&
                      get_power_status_led(
                          static_cast<TDeviceConfig_PowerStatusLed *>(ptr));
          break;
      }

      if (field_set) {
        config->Fields |= it->first;
        config->ConfigSize += size;
      } else if (size > left) {
        return;
      }

      if (fields_left) {
        *fields_left &= ~(it->first);
      }

      if (config->ConfigSize >= SUPLA_DEVICE_CONFIG_MAXSIZE) {
        return;
      }
    }
  }
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t *fields_left) {
  get_config(config, 0xFFFFFFFFFFFFFFFF, fields_left);
}

void device_json_config::remove_empty_sub_roots() {
  cJSON *root = get_user_root();
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *item = get_root(false, it->first);
    if (item && root != item && !item->child) {
      cJSON_DetachItemViaPointer(root, item);
      cJSON_Delete(item);
    }
  }
}

void device_json_config::leave_only_thise_fields(
    unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (!(fields & it->first)) {
      cJSON *root = get_root(false, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  if (!(fields & SUPLA_DEVICE_CONFIG_FIELD_HOME_SCREEN_CONTENT)) {
    cJSON *item = cJSON_GetObjectItem(get_properties_root(), content_available);
    if (item) {
      cJSON_DetachItemViaPointer(get_properties_root(), item);
      cJSON_Delete(item);
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::remove_fields(unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (fields & it->first) {
      cJSON *root = get_root(false, it->first);
      if (root) {
        cJSON *item = cJSON_GetObjectItem(root, it->second.c_str());
        if (item) {
          cJSON_DetachItemViaPointer(root, item);
          cJSON_Delete(item);
        }
      }
    }
  }

  remove_empty_sub_roots();
}

void device_json_config::merge(supla_json_config *_dst) {
  device_json_config dst(_dst);

  map<unsigned _supla_int16_t, string> props_fields = {{1, content_available}};
  map<cJSON *, map<unsigned _supla_int16_t, string>> map;

  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    cJSON *root = get_root(false, it->first);
    if (root) {
      map[root][it->first] = it->second;
    }
  }

  for (auto rit = map.cbegin(); rit != map.cend(); ++rit) {
    for (auto fit = rit->second.cbegin(); fit != rit->second.cend(); ++fit) {
      supla_json_config::merge(rit->first, dst.get_root(true, fit->first),
                               rit->second, false);
    }
  }

  remove_empty_sub_roots();

  supla_json_config::merge(get_properties_root(), dst.get_properties_root(),
                           props_fields, false);
}
