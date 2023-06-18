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

#ifndef DEVICE_JSON_CONFIG_H_
#define DEVICE_JSON_CONFIG_H_

#include <map>
#include <string>

#include "jsonconfig/json_config.h"
#include "proto.h"

class device_json_config : public supla_json_config {
 private:
  static const std::map<unsigned _supla_int16_t, std::string> field_map;

  std::string status_led_to_string(unsigned char status);
  unsigned char string_to_status_led(const std::string &status);
  std::string screen_saver_mode_to_string(unsigned char status);
  unsigned char string_to_screen_saver_mode(const std::string &status);

  void set_status_led(TDeviceConfig_StatusLed *status_led);
  void set_screen_brightness(TDeviceConfig_ScreenBrightness *brightness);
  void set_button_volume(TDeviceConfig_ButtonVolume *volume);
  void set_local_config_disabled(TDeviceConfig_DisableLocalConfig *disabled);
  void set_timezone_offset(TDeviceConfig_TimezoneOffset *offset);
  void set_automatic_time_sync(TDeviceConfig_AutomaticTimeSync *time_sync);
  void set_screen_saver_delay(TDeviceConfig_ScreensaverDelay *delay);
  void set_screen_saver_mode(TDeviceConfig_ScreensaverMode *mode);

 public:
  device_json_config(void);
  explicit device_json_config(supla_json_config *root);
  virtual ~device_json_config(void);
  void set_config(TSDS_SetDeviceConfig *config);
  void get_config(TSDS_SetDeviceConfig *config, unsigned _supla_int64_t fields,
                  unsigned _supla_int64_t *fields_left);
  void get_config(TSDS_SetDeviceConfig *config,
                  unsigned _supla_int64_t *fields_left);
  void leave_only_thise_fields(unsigned _supla_int64_t fields);
  void remove_fields(unsigned _supla_int64_t fields);
  virtual void merge(supla_json_config *dst);
  bool is_local_config_disabled(void);
  bool get_screen_brightness(TDeviceConfig_ScreenBrightness *brightness);
  bool get_button_volume(TDeviceConfig_ButtonVolume *volume);
  bool get_local_config_disabled(TDeviceConfig_DisableLocalConfig *disabled);
  bool get_timezone_offset(TDeviceConfig_TimezoneOffset *offset);
  bool get_automatic_time_sync(TDeviceConfig_AutomaticTimeSync *time_sync);
  bool get_screen_saver_delay(TDeviceConfig_ScreensaverDelay *delay);
  bool get_status_led(TDeviceConfig_StatusLed *status_led);
  bool get_screen_saver_mode(TDeviceConfig_ScreensaverMode *mode);
};

#endif /* DEVICE_JSON_CONFIG_H_ */
