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

#ifndef WEBHOOK_STATE_WEBHOOK_CLIENT2_H_
#define WEBHOOK_STATE_WEBHOOK_CLIENT2_H_

#include "channel_ic_measurement.h"
#include "device/channel_electricity_measurement.h"
#include "http/abstract_curl_adapter.h"
#include "json/cJSON.h"
#include "webhook/abstract_state_webhook_credentials.h"

class supla_state_webhook_client2 {
 private:
  int channel_id;
  bool channel_connected;
  __time_t timestamp;
  supla_abstract_curl_adapter *curl_adapter;
  supla_abstract_state_webhook_credentials *credentials;
  cJSON *get_header(const char *function);
  void refresh_token(void);
  bool perform_post_request(const char *fields, int *http_result_code);
  bool send_report(const char *json_string);
  bool send_report(cJSON *json);
  bool report_with_bool(const char *function, const char *name, bool _true);
  bool report_with_number(const char *function, const char *name,
                          double number);
  bool value_report(const char *function, double number);
  bool on_report(const char *function, bool on);
  bool hi_report(const char *function, bool hi);
  bool shut_report(const char *function, char shut);
  bool temperature_and_humidity_report(const char *function,
                                       double *temperature, double *humidity);
  bool dimmer_and_rgb_report(const char *function, int *color,
                             char *color_brightness, char *brightness, char on);
  bool impulse_counter_measurement_report(const char *function,
                                          supla_channel_ic_measurement *icm);

 public:
  explicit supla_state_webhook_client2(
      int channel_id, supla_abstract_curl_adapter *curl_adapter,
      supla_abstract_state_webhook_credentials *credentials);

  void set_channel_connected(bool connected);
  void set_timestamp(__time_t timestamp);
  bool power_switch_report(bool on);
  bool light_switch_report(bool on);
  bool staircase_timer_report(bool on);
  bool temperature_report(double temperature);
  bool humidity_report(double humidity);
  bool temperature_and_humidity_report(double temperature, double humidity);
  bool gateway_opening_sensor_report(bool hi);
  bool gate_opening_sensor_report(bool hi);
  bool garage_door_opening_sensor_report(bool hi);
  bool noliquid_sensor_report(bool hi);
  bool door_opening_sensor_report(bool hi);
  bool roller_shutter_opening_sensor_report(bool hi);
  bool roof_window_opening_sensor_report(bool hi);
  bool window_opening_sensor_report(bool hi);
  bool mail_sensor_report(bool hi);
  bool roller_shutter_report(char shut);
  bool roof_window_report(char shut);
  bool wind_sensor_report(double value);
  bool pressure_sensor_report(double value);
  bool rain_sensor_report(double value);
  bool weight_sensor_report(double value);
  bool distance_sensor_report(double distance);
  bool depth_sensor_report(double depth);
  bool dimmer_report(char brightness, char on);
  bool dimmer_and_rgb_report(int color, char color_brightness, char brightness,
                             char on);
  bool rgb_report(int color, char color_brightness, char on);
  bool electricity_measurement_report(
      supla_channel_electricity_measurement *em);
  bool impulse_counter_electricity_measurement_report(
      supla_channel_ic_measurement *icm);
  bool impulse_counter_gas_measurement_report(
      supla_channel_ic_measurement *icm);
  bool impulse_counter_water_measurement_report(
      supla_channel_ic_measurement *icm);
  bool impulse_counter_heat_measurement_report(
      supla_channel_ic_measurement *icm);
  bool triggered_actions_report(unsigned int actions);
};

#endif /* WEBHOOK_STATE_WEBHOOK_CLIENT2_H_ */
