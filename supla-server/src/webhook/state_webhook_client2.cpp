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
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_abstract_state_webhook_credentials *credentials) {
  this->channel_id = channel_id;
  this->online = false;
  this->curl_adapter = curl_adapter;
  this->credentials = credentials;

  struct timeval now;
  gettimeofday(&now, NULL);
  this->timestamp = now.tv_sec;
}

void supla_state_webhook_client2::set_online(bool online) {
  this->online = online;
}

void supla_state_webhook_client2::set_timestamp(__time_t timestamp) {
  this->timestamp = timestamp;
}

cJSON *supla_state_webhook_client2::get_header(const char *function) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    cJSON_AddStringToObject(header, "userShortUniqueId",
                            credentials->get_user_short_unique_id());

    cJSON_AddNumberToObject(header, "channelId", channel_id);

    cJSON_AddStringToObject(header, "channelFunction", function);

    cJSON_AddNumberToObject(header, "timestamp", timestamp);
  }

  return header;
}

bool supla_state_webhook_client2::send_report(cJSON *json) {}

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
    unsigned int actions) {
  if (!actions) {
    return false;
  }

  cJSON *root = get_header("ACTION_TRIGGER");
  if (root) {
    cJSON *json_actions = cJSON_CreateArray();
    if (json_actions) {
      unsigned int bit = 1;
      for (unsigned int a = 0; a < sizeof(actions) * 8; a++) {
        cJSON *item = NULL;

        if (actions & bit) {
          switch (bit) {
            case SUPLA_ACTION_CAP_TURN_ON:
              item = cJSON_CreateString("TURN_ON");
              break;
            case SUPLA_ACTION_CAP_TURN_OFF:
              item = cJSON_CreateString("TURN_OFF");
              break;
            case SUPLA_ACTION_CAP_TOGGLE_x1:
              item = cJSON_CreateString("TOGGLE_X1");
              break;
            case SUPLA_ACTION_CAP_TOGGLE_x2:
              item = cJSON_CreateString("TOGGLE_X2");
              break;
            case SUPLA_ACTION_CAP_TOGGLE_x3:
              item = cJSON_CreateString("TOGGLE_X3");
              break;
            case SUPLA_ACTION_CAP_TOGGLE_x4:
              item = cJSON_CreateString("TOGGLE_X4");
              break;
            case SUPLA_ACTION_CAP_TOGGLE_x5:
              item = cJSON_CreateString("TOGGLE_X5");
              break;
            case SUPLA_ACTION_CAP_HOLD:
              item = cJSON_CreateString("HOLD");
              break;
            case SUPLA_ACTION_CAP_SHORT_PRESS_x1:
              item = cJSON_CreateString("PRESS_X1");
              break;
            case SUPLA_ACTION_CAP_SHORT_PRESS_x2:
              item = cJSON_CreateString("PRESS_X2");
              break;
            case SUPLA_ACTION_CAP_SHORT_PRESS_x3:
              item = cJSON_CreateString("PRESS_X3");
              break;
            case SUPLA_ACTION_CAP_SHORT_PRESS_x4:
              item = cJSON_CreateString("PRESS_X4");
              break;
            case SUPLA_ACTION_CAP_SHORT_PRESS_x5:
              item = cJSON_CreateString("PRESS_X5");
              break;
          }

          if (item) {
            cJSON_AddItemToArray(json_actions, item);
          }
        }
        bit <<= 1;
      }

      cJSON_AddItemToObject(root, "triggered_actions", json_actions);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}
