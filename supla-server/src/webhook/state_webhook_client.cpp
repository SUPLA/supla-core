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

#include "state_webhook_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "device/extended_value/channel_em_extended_value.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_fb_value.h"
#include "device/value/channel_floating_point_sensor_value.h"
#include "device/value/channel_gate_value.h"
#include "device/value/channel_hvac_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "webhook/state_webhook_request.h"

using std::string;

supla_state_webhook_client::supla_state_webhook_client(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_state_webhook_credentials *credentials, __time_t timestamp) {
  this->channel_id = channel_id;
  this->channel_connected = false;
  this->curl_adapter = curl_adapter;
  this->credentials = credentials;
  this->channel_value = nullptr;
  this->channel_extended_value = nullptr;

  if (timestamp == 0) {
    struct timeval now;
    gettimeofday(&now, nullptr);
    timestamp = now.tv_sec;
  }

  this->timestamp = timestamp;
}

supla_state_webhook_client::~supla_state_webhook_client(void) {
  if (channel_value) {
    delete channel_value;
  }

  if (channel_extended_value) {
    delete channel_extended_value;
  }
}

void supla_state_webhook_client::set_channel_connected(bool connected) {
  this->channel_connected = connected;
}

void supla_state_webhook_client::set_channel_value(
    supla_abstract_channel_value *channel_value) {
  if (this->channel_value) {
    delete channel_value;
  }

  this->channel_value = channel_value;
}

void supla_state_webhook_client::set_channel_extended_value(
    supla_abstract_channel_extended_value *channel_extended_value) {
  if (this->channel_extended_value) {
    delete channel_extended_value;
  }

  this->channel_extended_value = channel_extended_value;
}

cJSON *supla_state_webhook_client::get_header(const char *function) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    cJSON_AddStringToObject(header, "userShortUniqueId",
                            credentials->get_user_short_unique_id().c_str());

    cJSON_AddNumberToObject(header, "channelId", channel_id);

    cJSON_AddStringToObject(header, "channelFunction", function);

    cJSON_AddNumberToObject(header, "timestamp", timestamp);
  }

  return header;
}

void supla_state_webhook_client::refresh_token(void) {
  if (!credentials->is_refresh_token_exists()) {
    return;
  }

  struct timeval last_set_time = credentials->get_set_time();
  credentials->refresh_lock();

  struct timeval current_set_time = credentials->get_set_time();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
    string refresh_token = credentials->get_refresh_token();

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "refreshToken", refresh_token.c_str());
    cJSON_AddStringToObject(root, "userShortUniqueId",
                            credentials->get_user_short_unique_id().c_str());
    char *str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    string request_result;

    curl_adapter->reset();
    curl_adapter->set_opt_url(credentials->get_url().c_str());
    curl_adapter->append_header("Content-Type: application/json");
    curl_adapter->set_opt_post_fields(str);
    curl_adapter->set_opt_custom_request("PUT");
    curl_adapter->set_opt_write_data(&request_result);
    curl_adapter->perform();

    free(str);

    if (curl_adapter->get_response_code() == 200 && request_result.size()) {
      cJSON *root = cJSON_Parse(request_result.c_str());
      if (root) {
        cJSON *access_token = cJSON_GetObjectItem(root, "accessToken");
        cJSON *expires_in = cJSON_GetObjectItem(root, "expiresIn");
        cJSON *refresh_token = cJSON_GetObjectItem(root, "refreshToken");

        if (access_token && cJSON_IsString(access_token) && refresh_token &&
            cJSON_IsString(refresh_token)) {
          int ex_in = cJSON_IsNumber(expires_in) ? expires_in->valueint : 0;
          credentials->update(cJSON_GetStringValue(access_token),
                              cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }
  }

  credentials->refresh_unlock();
}

bool supla_state_webhook_client::perform_post_request(const char *fields,
                                                      int *http_result_code) {
  if (http_result_code) {
    *http_result_code = 0;
  }

  if (!credentials->is_access_token_exists()) {
    return false;
  }

  string token = "Authorization: Bearer ";
  token.append(credentials->get_access_token());

  curl_adapter->reset();
  curl_adapter->set_opt_url(credentials->get_url().c_str());
  curl_adapter->append_header("Content-Type: application/json");
  curl_adapter->append_header(token.c_str());
  curl_adapter->set_opt_post_fields(fields);

  if (curl_adapter->perform()) {
    if (http_result_code) {
      *http_result_code = curl_adapter->get_response_code();
    }
    return curl_adapter->get_response_code() == 200 ||
           curl_adapter->get_response_code() == 202;
  }

  return false;
}

bool supla_state_webhook_client::send_report(const char *json_string) {
  if (!credentials->is_access_token_exists()) {
    return false;
  }

  bool refresh_attempt = false;

  if (credentials->expires_in() <= 30) {
    refresh_attempt = true;
    refresh_token();
  }

  int http_result_code = 0;
  if (perform_post_request(json_string, &http_result_code)) {
    return true;
  }

  if (!refresh_attempt && http_result_code == 403) {
    refresh_attempt = true;
    http_result_code = 0;
    refresh_token();
    if (perform_post_request(json_string, &http_result_code)) {
      return true;
    }
  }

  if (refresh_attempt && http_result_code == 403) {
    credentials->remove();
  }

  return false;
}

bool supla_state_webhook_client::send_report(cJSON *json) {
  if (json == nullptr) {
    return false;
  }

  bool result = false;

  char *json_string = cJSON_PrintUnformatted(json);

  cJSON_Delete(json);
  json = nullptr;

  if (json_string != NULL) {
    result = send_report(json_string);

    free(json_string);
    json_string = nullptr;
  }

  return result;
}

bool supla_state_webhook_client::report_with_bool(const char *function,
                                                  const char *name,
                                                  bool _true) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      cJSON_AddBoolToObject(state, name, _true);
      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::report_with_number(const char *function,
                                                    const char *name) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      supla_channel_floating_point_sensor_value *fp_value =
          dynamic_cast<supla_channel_floating_point_sensor_value *>(
              channel_value);

      cJSON_AddNumberToObject(state, name ? name : "value",
                              fp_value ? fp_value->get_value() : 0.00);

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::on_report(const char *function) {
  supla_channel_onoff_value *v =
      dynamic_cast<supla_channel_onoff_value *>(channel_value);

  return report_with_bool(function, "on", v && v->is_on());
}

bool supla_state_webhook_client::hi_report(const char *function) {
  supla_channel_binary_sensor_value *v =
      dynamic_cast<supla_channel_binary_sensor_value *>(channel_value);

  return report_with_bool(function, "hi", v && v->is_hi());
}

bool supla_state_webhook_client::power_switch_report(void) {
  return on_report("POWERSWITCH");
}

bool supla_state_webhook_client::light_switch_report() {
  return on_report("LIGHTSWITCH");
}

bool supla_state_webhook_client::staircase_timer_report() {
  return on_report("STAIRCASETIMER");
}

bool supla_state_webhook_client::temperature_and_humidity_report(
    const char *function, bool temperature, bool humidity) {
  supla_channel_temphum_value *temphum =
      dynamic_cast<supla_channel_temphum_value *>(channel_value);

  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (temperature) {
        cJSON_AddNumberToObject(
            state, "temperature",
            temphum ? temphum->get_temperature()
                    : supla_channel_temphum_value::incorrect_temperature());
      }

      if (humidity) {
        cJSON_AddNumberToObject(
            state, "humidity",
            temphum ? temphum->get_humidity()
                    : supla_channel_temphum_value::incorrect_humidity());
      }

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::temperature_report() {
  return temperature_and_humidity_report("THERMOMETER", true, false);
}

bool supla_state_webhook_client::humidity_report() {
  return temperature_and_humidity_report("HUMIDITY", false, true);
}

bool supla_state_webhook_client::temperature_and_humidity_report() {
  return temperature_and_humidity_report("HUMIDITYANDTEMPERATURE", true, true);
}

bool supla_state_webhook_client::gateway_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_GATEWAY");
}

bool supla_state_webhook_client::gate_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_GATE");
}

bool supla_state_webhook_client::garage_door_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_GARAGEDOOR");
}

bool supla_state_webhook_client::noliquid_sensor_report() {
  return hi_report("NOLIQUIDSENSOR");
}

bool supla_state_webhook_client::door_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_DOOR");
}

bool supla_state_webhook_client::roller_shutter_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_ROLLERSHUTTER");
}

bool supla_state_webhook_client::roof_window_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_ROOFWINDOW");
}

bool supla_state_webhook_client::window_opening_sensor_report() {
  return hi_report("OPENINGSENSOR_WINDOW");
}

bool supla_state_webhook_client::hotel_card_sensor_report() {
  return hi_report("HOTELCARDSENSOR");
}

bool supla_state_webhook_client::mail_sensor_report() {
  return hi_report("MAILSENSOR");
}

bool supla_state_webhook_client::shut_report(const char *function) {
  supla_channel_rs_value *rs_val =
      dynamic_cast<supla_channel_rs_value *>(channel_value);

  supla_channel_fb_value *fb_val = nullptr;

  if (!rs_val) {
    fb_val = dynamic_cast<supla_channel_fb_value *>(channel_value);
  }

  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      char shut = 0;
      if (rs_val) {
        shut = rs_val->get_rs_value()->position;
      } else if (fb_val) {
        shut = fb_val->get_position();
        cJSON_AddNumberToObject(
            state, "tilt", fb_val->get_tilt() >= 0 ? fb_val->get_tilt() : 0);
      }

      cJSON_AddNumberToObject(state, "shut", shut >= 0 ? shut : 0);
      cJSON_AddBoolToObject(state, "is_calibrating", shut < 0);

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::wind_sensor_report() {
  return report_with_number("WINDSENSOR", nullptr);
}

bool supla_state_webhook_client::pressure_sensor_report() {
  return report_with_number("PRESSURESENSOR", nullptr);
}

bool supla_state_webhook_client::rain_sensor_report() {
  return report_with_number("RAINSENSOR", nullptr);
}

bool supla_state_webhook_client::weight_sensor_report() {
  return report_with_number("WEIGHTSENSOR", nullptr);
}

bool supla_state_webhook_client::distance_sensor_report() {
  return report_with_number("DISTANCESENSOR", "distance");
}

bool supla_state_webhook_client::depth_sensor_report() {
  return report_with_number("DEPTHSENSOR", "depth");
}

bool supla_state_webhook_client::dimmer_and_rgb_report(const char *function,
                                                       bool rgb, bool white) {
  supla_channel_rgbw_value *rgbw =
      dynamic_cast<supla_channel_rgbw_value *>(channel_value);

  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      char color_hex[9] = {};
      unsigned int color = 0;
      char brightness = 0;
      char color_brightness = 0;

      if (rgbw) {
        color = rgbw->get_color();
        brightness = rgbw->get_brightness();
        color_brightness = rgbw->get_color_brightness();
      }

      if (rgb) {
        snprintf(color_hex, sizeof(color_hex), "0x%02X%02X%02X",
                 ((unsigned char *)&color)[2], ((unsigned char *)&color)[1],
                 ((unsigned char *)&color)[0]);
        cJSON_AddStringToObject(state, "color", color_hex);

        cJSON_AddNumberToObject(state, "color_brightness", color_brightness);
      }

      if (white) {
        cJSON_AddNumberToObject(state, "brightness", brightness);
      }

      cJSON_AddBoolToObject(
          state, "on",
          (white && brightness > 0) || (rgb && color_brightness > 0));

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::dimmer_report() {
  return dimmer_and_rgb_report("DIMMER", false, true);
}

bool supla_state_webhook_client::dimmer_and_rgb_report() {
  return dimmer_and_rgb_report("DIMMERANDRGBLIGHTING", true, true);
}

bool supla_state_webhook_client::rgb_report() {
  return dimmer_and_rgb_report("RGBLIGHTING", true, false);
}

bool supla_state_webhook_client::electricity_measurement_report(void) {
  cJSON *root = get_header("ELECTRICITYMETER");
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      supla_channel_em_extended_value *emv =
          dynamic_cast<supla_channel_em_extended_value *>(
              channel_extended_value);

      if (channel_connected && emv != nullptr) {
        TElectricityMeter_ExtendedValue_V3 em_ev;
        emv->get_raw_value(&em_ev);

        if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
            !(em_ev.measured_values & EM_VAR_CURRENT)) {
          em_ev.m[0].current[0] *= 10;
          em_ev.m[0].current[1] *= 10;
          em_ev.m[0].current[2] *= 10;
        }

        cJSON_AddNumberToObject(state, "support", em_ev.measured_values);

        if (emv->get_currency().empty()) {
          cJSON_AddNullToObject(state, "currency");
        } else {
          cJSON_AddStringToObject(state, "currency",
                                  emv->get_currency().c_str());
        }

        cJSON_AddNumberToObject(state, "pricePerUnit",
                                em_ev.price_per_unit * 0.0001);

        cJSON_AddNumberToObject(state, "totalCost", em_ev.total_cost * 0.01);

        cJSON *phases = cJSON_AddArrayToObject(state, "phases");
        if (phases) {
          for (int p = 0; p < 3; p++) {
            cJSON *phase = cJSON_CreateObject();
            if (phase) {
              cJSON_AddNumberToObject(phase, "number", p + 1);

              if (em_ev.m_count > 0) {
                if (em_ev.measured_values & EM_VAR_FREQ) {
                  cJSON_AddNumberToObject(phase, "frequency",
                                          em_ev.m[0].freq * 0.01);
                }

                if (em_ev.measured_values & EM_VAR_VOLTAGE) {
                  cJSON_AddNumberToObject(phase, "voltage",
                                          em_ev.m[0].voltage[p] * 0.01);
                }

                if (em_ev.measured_values & EM_VAR_CURRENT) {
                  cJSON_AddNumberToObject(phase, "current",
                                          em_ev.m[0].current[p] * 0.001);
                }

                if (em_ev.measured_values & EM_VAR_POWER_ACTIVE) {
                  double value = em_ev.m[0].power_active[p] * 0.00001;
                  if (em_ev.measured_values & EM_VAR_POWER_ACTIVE_KW) {
                    value *= 1000;
                  }
                  cJSON_AddNumberToObject(phase, "powerActive", value);
                }

                if (em_ev.measured_values & EM_VAR_POWER_REACTIVE) {
                  double value = em_ev.m[0].power_reactive[p] * 0.00001;
                  if (em_ev.measured_values & EM_VAR_POWER_REACTIVE_KVAR) {
                    value *= 1000;
                  }
                  cJSON_AddNumberToObject(phase, "powerReactive", value);
                }

                if (em_ev.measured_values & EM_VAR_POWER_APPARENT) {
                  double value = em_ev.m[0].power_apparent[p] * 0.00001;
                  if (em_ev.measured_values & EM_VAR_POWER_APPARENT_KVA) {
                    value *= 1000;
                  }
                  cJSON_AddNumberToObject(phase, "powerApparent", value);
                }

                if (em_ev.measured_values & EM_VAR_POWER_FACTOR) {
                  cJSON_AddNumberToObject(phase, "powerFactor",
                                          em_ev.m[0].power_factor[p] * 0.001);
                }

                if (em_ev.measured_values & EM_VAR_PHASE_ANGLE) {
                  cJSON_AddNumberToObject(phase, "phaseAngle",
                                          em_ev.m[0].phase_angle[p] * 0.1);
                }
              }

              if (em_ev.measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY) {
                cJSON_AddNumberToObject(
                    phase, "totalForwardActiveEnergy",
                    em_ev.total_forward_active_energy[p] * 0.00001);
              }

              if (em_ev.measured_values & EM_VAR_REVERSE_ACTIVE_ENERGY) {
                cJSON_AddNumberToObject(
                    phase, "totalReverseActiveEnergy",
                    em_ev.total_reverse_active_energy[p] * 0.00001);
              }

              if (em_ev.measured_values & EM_VAR_FORWARD_REACTIVE_ENERGY) {
                cJSON_AddNumberToObject(
                    phase, "totalForwardReactiveEnergy",
                    em_ev.total_forward_reactive_energy[p] * 0.00001);
              }

              if (em_ev.measured_values & EM_VAR_REVERSE_REACTIVE_ENERGY) {
                cJSON_AddNumberToObject(
                    phase, "totalReverseReactiveEnergy",
                    em_ev.total_reverse_reactive_energy[p] * 0.00001);
              }

              cJSON_AddItemToArray(phases, phase);
            }
          }
        }

        if (em_ev.measured_values & EM_VAR_FORWARD_ACTIVE_ENERGY_BALANCED) {
          cJSON_AddNumberToObject(
              state, "totalForwardActiveEnergyBalanced",
              em_ev.total_forward_active_energy_balanced * 0.00001);
        }

        if (em_ev.measured_values & EM_VAR_REVERSE_ACTIVE_ENERGY_BALANCED) {
          cJSON_AddNumberToObject(
              state, "totalReverseActiveEnergyBalanced",
              em_ev.total_reverse_active_energy_balanced * 0.00001);
        }
      }

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::impulse_counter_measurement_report(
    const char *function) {
  supla_channel_ic_extended_value *icv =
      dynamic_cast<supla_channel_ic_extended_value *>(channel_extended_value);

  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (channel_connected && icv != nullptr) {
        cJSON_AddNumberToObject(state, "totalCost",
                                icv->get_total_cost() * 0.01);
        cJSON_AddNumberToObject(state, "pricePerUnit",
                                icv->get_price_per_unit() * 0.0001);
        cJSON_AddNumberToObject(state, "impulsesPerUnit",
                                icv->get_impulses_per_unit());
        cJSON_AddNumberToObject(state, "counter", icv->get_counter());
        cJSON_AddNumberToObject(state, "calculatedValue",
                                icv->get_calculated_value() * 0.001);
        if (icv->get_currency().size() == 0) {
          cJSON_AddNullToObject(state, "currency");
        } else {
          cJSON_AddStringToObject(state, "currency",
                                  icv->get_currency().c_str());
        }

        if (icv->get_custom_unit().size() == 0) {
          cJSON_AddNullToObject(state, "unit");
        } else {
          cJSON_AddStringToObject(state, "unit",
                                  icv->get_custom_unit().c_str());
        }
      }

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::impulse_counter_electricity_measurement_report(
    void) {
  return impulse_counter_measurement_report("IC_ELECTRICITYMETER");
}

bool supla_state_webhook_client::impulse_counter_gas_measurement_report(void) {
  return impulse_counter_measurement_report("IC_GASMETER");
}

bool supla_state_webhook_client::impulse_counter_water_measurement_report(
    void) {
  return impulse_counter_measurement_report("IC_WATERMETER");
}

bool supla_state_webhook_client::impulse_counter_heat_measurement_report(void) {
  return impulse_counter_measurement_report("IC_HEATMETER");
}

bool supla_state_webhook_client::triggered_actions_report(
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
        cJSON *item = nullptr;

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

bool supla_state_webhook_client::hvac_report(const char *function) {
  supla_channel_hvac_value *hvac_val =
      dynamic_cast<supla_channel_hvac_value *>(channel_value);

  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (channel_connected && hvac_val != nullptr) {
        if (hvac_val->is_on() < 2) {
          cJSON_AddBoolToObject(state, "is_on",
                                hvac_val->is_on() ? cJSON_True : cJSON_False);
        } else {
          cJSON_AddNumberToObject(state, "is_on", hvac_val->is_on() - 2);
        }

        string mode;
        switch (hvac_val->get_mode()) {
          case SUPLA_HVAC_MODE_NOT_SET:
            mode = "NOT_SET";
            break;
          case SUPLA_HVAC_MODE_OFF:
            mode = "OFF";
            break;
          case SUPLA_HVAC_MODE_HEAT:
            mode = "HEAT";
            break;
          case SUPLA_HVAC_MODE_COOL:
            mode = "COOL";
            break;
          case SUPLA_HVAC_MODE_HEAT_COOL:
            mode = "HEAT_COOL";
            break;
          case SUPLA_HVAC_MODE_FAN_ONLY:
            mode = "FAN_ONLY";
            break;
          case SUPLA_HVAC_MODE_DRY:
            mode = "DRY";
            break;
        }

        cJSON_AddStringToObject(state, "mode", mode.c_str());

        if (hvac_val->get_flags() &
            SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_HEAT_SET) {
          cJSON_AddNumberToObject(
              state, "setpoint_temperature_heat",
              hvac_val->get_setpoint_temperature_heat_dbl());
        }

        if (hvac_val->get_flags() &
            SUPLA_HVAC_VALUE_FLAG_SETPOINT_TEMP_COOL_SET) {
          cJSON_AddNumberToObject(
              state, "setpoint_temperature_cool",
              hvac_val->get_setpoint_temperature_cool_dbl());
        }

        cJSON *flags = cJSON_CreateArray();

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_HEATING) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("HEATING"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_COOLING) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("COOLING"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("WEEKLY_SCHEDULE"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_COUNTDOWN_TIMER) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("COUNTDOWN_TIMER"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_FAN_ENABLED) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("FAN_ENABLED"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_THERMOMETER_ERROR) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("THERMOMETER_ERROR"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_CLOCK_ERROR) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("CLOCK_ERROR"));
        }

        if (hvac_val->get_flags() &
            SUPLA_HVAC_VALUE_FLAG_FORCED_OFF_BY_SENSOR) {
          cJSON_AddItemToArray(flags,
                               cJSON_CreateString("FORCED_OFF_BY_SENSOR"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_COOL) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("COOL"));
        }

        if (hvac_val->get_flags() &
            SUPLA_HVAC_VALUE_FLAG_WEEKLY_SCHEDULE_TEMPORAL_OVERRIDE) {
          cJSON_AddItemToArray(
              flags, cJSON_CreateString("WEEKLY_SCHEDULE_TEMPORAL_OVERRIDE"));
        }

        if (hvac_val->get_flags() & SUPLA_HVAC_VALUE_FLAG_BATTERY_COVER_OPEN) {
          cJSON_AddItemToArray(flags, cJSON_CreateString("BATTERY_COVER_OPEN"));
        }

        cJSON_AddItemToObject(state, "flags", flags);
      }

      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}
