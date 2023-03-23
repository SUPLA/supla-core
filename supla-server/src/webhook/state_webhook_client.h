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

#ifndef WEBHOOK_STATE_WEBHOOK_CLIENT_H_
#define WEBHOOK_STATE_WEBHOOK_CLIENT_H_

#include <webhook/state_webhook_credentials.h>
#include "channel_ic_measurement.h"
#include "device/channel_electricity_measurement.h"
#include "device/value/channel_value.h"
#include "http/abstract_curl_adapter.h"
#include "json/cJSON.h"

class supla_state_webhook_client {
 private:
  int channel_id;
  bool channel_connected;
  supla_abstract_curl_adapter *curl_adapter;
  supla_state_webhook_credentials *credentials;
  supla_channel_value *channel_value;

  cJSON *get_header(const char *function);
  void refresh_token(void);
  bool perform_post_request(const char *fields, int *http_result_code);
  bool send_report(const char *json_string);
  bool send_report(cJSON *json);
  bool report_with_bool(const char *function, const char *name, bool _true);
  bool report_with_number(const char *function, const char *name);
  bool on_report(const char *function);
  bool hi_report(const char *function);
  bool shut_report(const char *function);
  bool temperature_and_humidity_report(const char *function, bool temperature,
                                       bool humidity);
  bool dimmer_and_rgb_report(const char *function, bool rgb, bool white);
  bool impulse_counter_measurement_report(const char *function,
                                          supla_channel_ic_measurement *icm);

 public:
  explicit supla_state_webhook_client(
      int channel_id, supla_abstract_curl_adapter *curl_adapter,
      supla_state_webhook_credentials *credentials);

  void set_channel_connected(bool connected);
  void set_channel_value(supla_channel_value *channel_value);

  bool power_switch_report(void);
  bool light_switch_report(void);
  bool staircase_timer_report(void);
  bool temperature_report(void);
  bool humidity_report(void);
  bool temperature_and_humidity_report(void);
  bool gateway_opening_sensor_report(void);
  bool gate_opening_sensor_report(void);
  bool garage_door_opening_sensor_report(void);
  bool noliquid_sensor_report(void);
  bool door_opening_sensor_report(void);
  bool roller_shutter_opening_sensor_report(void);
  bool roof_window_opening_sensor_report(void);
  bool window_opening_sensor_report(void);
  bool mail_sensor_report(void);
  bool roller_shutter_report(void);
  bool roof_window_report(void);
  bool wind_sensor_report();
  bool pressure_sensor_report();
  bool rain_sensor_report();
  bool weight_sensor_report();
  bool distance_sensor_report();
  bool depth_sensor_report();
  bool dimmer_report();
  bool dimmer_and_rgb_report();
  bool rgb_report();
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

#endif /* WEBHOOK_STATE_WEBHOOK_CLIENT_H_ */
