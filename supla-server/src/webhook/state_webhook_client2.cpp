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

#include <webhook/state_webhook_client2.h>

supla_state_webhook_client2::supla_state_webhook_client2(
    int channel_id, supla_abstract_curl_adapter *curl_adapter) {
  this->channel_id = channel_id;
  this->online = false;
  this->curl_adapter = curl_adapter;
}

void supla_state_webhook_client2::set_online(bool online) {
  this->online = online;
}

bool supla_state_webhook_client2::power_switch_report(bool on) {}

bool supla_state_webhook_client2::light_switch_report(bool on) {}

bool supla_state_webhook_client2::staircase_timer_report(bool on) {}

bool supla_state_webhook_client2::temperature_report(double temperature) {}

bool supla_state_webhook_client2::humidity_report(double humidity) {}

bool supla_state_webhook_client2::temperature_and_humidity_report(
    double temperature, double humidity) {}

bool supla_state_webhook_client2::gateway_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::gate_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::garage_door_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::noliquid_sensor_report(bool hi) {}

bool supla_state_webhook_client2::door_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::roller_shutter_opening_sensor_report(
    bool hi) {}

bool supla_state_webhook_client2::roof_window_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::window_opening_sensor_report(bool hi) {}

bool supla_state_webhook_client2::mail_sensor_report(bool hi) {}

bool supla_state_webhook_client2::roller_shutter_report(char shut) {}

bool supla_state_webhook_client2::roof_window_report(char shut) {}

bool supla_state_webhook_client2::wind_sensor_report(double value) {}

bool supla_state_webhook_client2::pressure_sensor_report(double value) {}

bool supla_state_webhook_client2::rain_sensor_report(double value) {}

bool supla_state_webhook_client2::weight_sensor_report(double value) {}

bool supla_state_webhook_client2::distance_sensor_report(double distance) {}

bool supla_state_webhook_client2::depth_sensor_report(double depth) {}

bool supla_state_webhook_client2::dimmer_report(char brightness, char on) {}
bool supla_state_webhook_client2::dimmer_and_rgb_report(int color,
                                                        char color_brightness,
                                                        char brightness,
                                                        char on) {}
bool supla_state_webhook_client2::rgb_report(int color, char color_brightness,
                                             char on) {}
bool supla_state_webhook_client2::electricity_measurement_report(
    supla_channel_electricity_measurement *em) {}
bool supla_state_webhook_client2::
    impulse_counter_electricity_measurement_report(
        supla_channel_ic_measurement *icm) {}
bool supla_state_webhook_client2::impulse_counter_gas_measurement_report(
    supla_channel_ic_measurement *icm) {}
bool supla_state_webhook_client2::impulse_counter_water_measurement_report(
    supla_channel_ic_measurement *icm) {}
bool supla_state_webhook_client2::impulse_counter_heat_measurement_report(
    supla_channel_ic_measurement *icm) {}
bool supla_state_webhook_client2::triggered_actions_report(
    unsigned int actions) {}
