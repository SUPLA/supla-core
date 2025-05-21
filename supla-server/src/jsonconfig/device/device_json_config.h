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
  static const std::map<unsigned _supla_int16_t, std::string>
      home_screen_content_map;
  static const std::map<unsigned _supla_int16_t, std::string> modbus_field_map;
  static const char content_available[];
  static const char disabled_str[];
  static const char min_allowed_temperature[];
  static const char max_allowed_temperature[];
  static const char level_str[];
  static const char adjustment_str[];

  std::string status_led_to_string(unsigned char status);
  unsigned char string_to_status_led(const std::string &status);
  std::string power_status_led_to_string(unsigned char status);
  unsigned char string_to_power_status_led(const std::string &status);
  std::string home_screen_content_to_string(unsigned char status);
  unsigned char string_to_home_screen_content(const std::string &status);
  std::string modbus_role_to_string(unsigned char role);
  unsigned char string_to_modbus_role(const std::string &role);
  std::string modbus_serial_mode_to_string(unsigned char mode);
  unsigned char string_to_modbus_serial_mode(const std::string &mode);

  std::string modbus_network_mode_to_string(unsigned char mode);
  unsigned char string_to_modbus_network_mode(const std::string &mode);

  std::string modbus_stop_bits_to_string(unsigned char stop_bits);
  unsigned char string_to_modbus_stop_bits(const std::string &stop_bits);

  cJSON *modbus_serial_config_to_json(ModbusSerialConfig *cfg);
  bool modbus_json_to_serial_config(cJSON *json, ModbusSerialConfig *cfg);
  cJSON *modbus_network_config_to_json(ModbusNetworkConfig *cfg);
  bool modbus_json_to_network_config(cJSON *json, ModbusNetworkConfig *cfg);

  cJSON *modbus_available_protocols_to_json(ModbusConfigProperties *props);
  bool modbus_json_to_available_protocols(cJSON *json,
                                          ModbusConfigProperties *props);
  cJSON *modbus_available_baudrates_to_json(ModbusConfigProperties *props);
  bool modbus_json_to_available_baudrates(cJSON *json,
                                          ModbusConfigProperties *props);
  cJSON *modbus_available_stop_bits_to_json(ModbusConfigProperties *props);
  bool modbus_json_to_available_stop_bits(cJSON *json,
                                          ModbusConfigProperties *props);

  void set_status_led(TDeviceConfig_StatusLed *status_led);
  void set_power_status_led(TDeviceConfig_PowerStatusLed *status_led);
  void set_screen_brightness(TDeviceConfig_ScreenBrightness *brightness);
  void set_button_volume(TDeviceConfig_ButtonVolume *volume);
  void set_user_interface_disabled(
      TDeviceConfig_DisableUserInterface *disabled);
  void set_automatic_time_sync(TDeviceConfig_AutomaticTimeSync *time_sync);
  cJSON *get_root(bool create, bool user, unsigned _supla_int64_t field);
  cJSON *get_modbus_root(bool create, bool user);
  void set_home_screen_off_delay(TDeviceConfig_HomeScreenOffDelay *delay);
  void set_home_screen_off_delay_type(
      TDeviceConfig_HomeScreenOffDelayType *type);
  void set_home_screen_content(TDeviceConfig_HomeScreenContent *content);
  void set_modbus_config(TDeviceConfig_Modbus *cfg);
  void remove_empty_sub_roots();

 public:
  device_json_config(void);
  explicit device_json_config(supla_json_config *root);
  virtual ~device_json_config(void);
  void set_config(TSDS_SetDeviceConfig *config);
  unsigned _supla_int64_t get_available_fields(void);
  void get_config(TSDS_SetDeviceConfig *config, unsigned _supla_int64_t fields,
                  unsigned _supla_int64_t *fields_left);
  void get_config(TSDS_SetDeviceConfig *config,
                  unsigned _supla_int64_t *fields_left);
  void leave_only_thise_fields(unsigned _supla_int64_t fields);
  void remove_fields(unsigned _supla_int64_t fields);
  virtual void merge(supla_json_config *dst);
  bool get_screen_brightness(TDeviceConfig_ScreenBrightness *brightness);
  bool get_button_volume(TDeviceConfig_ButtonVolume *volume);
  bool get_user_interface_disabled(
      TDeviceConfig_DisableUserInterface *disabled);
  bool get_automatic_time_sync(TDeviceConfig_AutomaticTimeSync *time_sync);
  bool get_home_screen_off_delay(TDeviceConfig_HomeScreenOffDelay *delay);
  bool get_status_led(TDeviceConfig_StatusLed *status_led);
  bool get_power_status_led(TDeviceConfig_PowerStatusLed *status_led);
  bool get_home_screen_content(TDeviceConfig_HomeScreenContent *content);
  bool get_home_screen_off_delay_type(
      TDeviceConfig_HomeScreenOffDelayType *type);
  bool get_modbus_config(TDeviceConfig_Modbus *cfg);
};

#endif /* DEVICE_JSON_CONFIG_H_ */
