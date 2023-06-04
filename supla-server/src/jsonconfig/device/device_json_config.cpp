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

using std::map;
using std::string;

const map<unsigned _supla_int16_t, string> device_json_config::field_map = {
    {SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED, "statusLed"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS, "screenBrightness"},
    {SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME, "buttonVolume"},
    {SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG, "localConfigDisabled"},
    {SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET, "timezoneOffset"},
    {SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC, "automaticTimeSync"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY, "screenSaverDelay"},
    {SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE, "screenSaverMode"}};

device_json_config::device_json_config(void) : supla_json_config() {}

device_json_config::~device_json_config(void) {}

void device_json_config::set_status_led(TDeviceConfig_StatusLed *status_led) {
  if (status_led && status_led->StatusLedType >= 0 &&
      status_led->StatusLedType <= 2) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED),
                   cJSON_Number, true, nullptr, status_led->StatusLedType);
  }
}

void device_json_config::set_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  if (brightness) {
    if (brightness->Automatic == 1) {
      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_String, true, "auto", 0);
    } else if (brightness->ScreenBrightness >= 0 &&
               brightness->ScreenBrightness <= 100) {
      set_item_value(get_user_root(),
                     field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS),
                     cJSON_Number, true, nullptr, brightness->ScreenBrightness);
    }
  }
}

void device_json_config::set_button_volume(TDeviceConfig_ButtonVolume *volume) {
  unsigned char v = volume ? volume->Volume : 0;

  if (v <= 100) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_BUTTON_VOLUME),
                   cJSON_Number, true, nullptr, v);
  }
}

void device_json_config::set_local_config_disabled(
    TDeviceConfig_DisableLocalConfig *disabled) {
  if (disabled && (disabled->DisableLocalConfig == 0 ||
                   disabled->DisableLocalConfig == 1)) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG),
                   disabled->DisableLocalConfig ? cJSON_True : cJSON_False,
                   true, nullptr, 0);
  }
}

void device_json_config::set_timezone_offset(
    TDeviceConfig_TimezoneOffset *offset) {
  if (offset && offset->TimezoneOffsetMinutes >= -1560 &&
      offset->TimezoneOffsetMinutes <= 1560) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET),
                   cJSON_Number, true, nullptr, offset->TimezoneOffsetMinutes);
  }
}

void device_json_config::set_automatic_time_sync(
    TDeviceConfig_AutomaticTimeSync *time_sync) {
  if (time_sync && (time_sync->AutomaticTimeSync == 0 ||
                    time_sync->AutomaticTimeSync == 1)) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC),
                   time_sync->AutomaticTimeSync ? cJSON_True : cJSON_False,
                   true, nullptr, 0);
  }
}

void device_json_config::set_screen_saver_delay(
    TDeviceConfig_ScreensaverDelay *delay) {
  if (delay) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY),
                   cJSON_Number, true, nullptr, delay->ScreensaverDelayMs);
  }
}

void device_json_config::set_screen_saver_mode(
    TDeviceConfig_ScreensaverMode *mode) {
  if (mode && mode->ScreensaverMode >= 0 && mode->ScreensaverMode <= 3) {
    set_item_value(get_user_root(),
                   field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE),
                   cJSON_Number, true, nullptr, mode->ScreensaverMode);
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
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG:
          if (left >= (size = sizeof(TDeviceConfig_DisableLocalConfig))) {
            set_local_config_disabled(
                static_cast<TDeviceConfig_DisableLocalConfig *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET:
          if (left >= (size = sizeof(TDeviceConfig_TimezoneOffset))) {
            set_timezone_offset(
                static_cast<TDeviceConfig_TimezoneOffset *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          if (left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync))) {
            set_automatic_time_sync(
                static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY:
          if (left >= (size = sizeof(TDeviceConfig_ScreensaverDelay))) {
            set_screen_saver_delay(
                static_cast<TDeviceConfig_ScreensaverDelay *>(ptr));
          }
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE:
          if (left >= (size = sizeof(TDeviceConfig_ScreensaverMode))) {
            set_screen_saver_mode(
                static_cast<TDeviceConfig_ScreensaverMode *>(ptr));
          }
          break;
      }

      offset += size;
    }
  }
}

bool device_json_config::get_status_led(TDeviceConfig_StatusLed *status_led) {
  double value = 0;
  if (status_led &&
      get_double(get_user_root(),
                 field_map.at(SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED).c_str(),
                 &value) &&
      value >= 0 && value <= 2) {
    status_led->StatusLedType = value;
    return true;
  }

  return false;
}

bool device_json_config::get_screen_brightness(
    TDeviceConfig_ScreenBrightness *brightness) {
  double value = 0;
  if (brightness &&
      get_double(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str(),
          &value) &&
      value >= 0 && value <= 100) {
    brightness->Automatic = 0;
    brightness->ScreenBrightness = value;
    return true;
  }

  std::string str_value;
  if (brightness &&
      get_string(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREEN_BRIGHTNESS).c_str(),
          &str_value) &&
      str_value == "auto") {
    brightness->Automatic = 1;
    brightness->ScreenBrightness = 0;
    return true;
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

bool device_json_config::get_local_config_disabled(
    TDeviceConfig_DisableLocalConfig *disabled) {
  bool value = 0;
  if (disabled &&
      get_bool(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG).c_str(),
          &value)) {
    disabled->DisableLocalConfig = value ? 1 : 0;
    return true;
  }

  return false;
}

bool device_json_config::get_timezone_offset(
    TDeviceConfig_TimezoneOffset *offset) {
  double value = 0;
  if (offset &&
      get_double(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET).c_str(),
          &value) &&
      value >= -1560 && value <= 1560) {
    offset->TimezoneOffsetMinutes = value;
    return true;
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

bool device_json_config::get_screen_saver_delay(
    TDeviceConfig_ScreensaverDelay *delay) {
  double value = 0;
  if (delay &&
      get_double(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY).c_str(),
          &value)) {
    delay->ScreensaverDelayMs = value;
    return true;
  }

  return false;
}

bool device_json_config::get_screen_saver_mode(
    TDeviceConfig_ScreensaverMode *mode) {
  double value = 0;
  if (mode &&
      get_double(
          get_user_root(),
          field_map.at(SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE).c_str(),
          &value) &&
      value >= 0 && value <= 3) {
    mode->ScreensaverMode = value;
    return true;
  }

  return false;
}

void device_json_config::get_config(TSDS_SetDeviceConfig *config,
                                    unsigned _supla_int64_t fields,
                                    unsigned _supla_int64_t *fields_left) {
  if (!config) {
    return;
  }

  config->Fields = 0;

  if (fields_left) {
    *fields_left = 0;
    // map is sorted
    for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
      if (fields & it->first) {
        *fields_left |= it->first;
      }
    }
  }

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
        case SUPLA_DEVICE_CONFIG_FIELD_DISABLE_LOCAL_CONFIG:
          field_set =
              left >= (size = sizeof(TDeviceConfig_DisableLocalConfig)) &&
              get_local_config_disabled(
                  static_cast<TDeviceConfig_DisableLocalConfig *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_TIMEZONE_OFFSET:
          field_set = left >= (size = sizeof(TDeviceConfig_TimezoneOffset)) &&
                      get_timezone_offset(
                          static_cast<TDeviceConfig_TimezoneOffset *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_AUTOMATIC_TIME_SYNC:
          field_set =
              left >= (size = sizeof(TDeviceConfig_AutomaticTimeSync)) &&
              get_automatic_time_sync(
                  static_cast<TDeviceConfig_AutomaticTimeSync *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_DELAY:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreensaverDelay)) &&
                      get_screen_saver_delay(
                          static_cast<TDeviceConfig_ScreensaverDelay *>(ptr));
          break;
        case SUPLA_DEVICE_CONFIG_FIELD_SCREENSAVER_MODE:
          field_set = left >= (size = sizeof(TDeviceConfig_ScreensaverMode)) &&
                      get_screen_saver_mode(
                          static_cast<TDeviceConfig_ScreensaverMode *>(ptr));
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

void device_json_config::leave_only_thise_fields(
    unsigned _supla_int64_t fields) {
  for (auto it = field_map.cbegin(); it != field_map.cend(); ++it) {
    if (!(fields & it->first)) {
      cJSON *item = cJSON_GetObjectItem(get_user_root(), it->second.c_str());
      if (item) {
        cJSON_DetachItemViaPointer(get_user_root(), item);
        cJSON_Delete(item);
      }
    }
  }
}

void device_json_config::merge(device_json_config *dst) {
  if (dst) {
    supla_json_config::merge(get_user_root(), dst->get_user_root(), field_map);
  }
}
