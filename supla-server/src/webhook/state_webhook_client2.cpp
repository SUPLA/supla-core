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

#include "webhook/state_webhook_client2.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using std::string;

supla_state_webhook_client2::supla_state_webhook_client2(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_abstract_state_webhook_credentials *credentials) {
  this->channel_id = channel_id;
  this->channel_connected = false;
  this->curl_adapter = curl_adapter;
  this->credentials = credentials;

  struct timeval now;
  gettimeofday(&now, nullptr);
  this->timestamp = now.tv_sec;
}

void supla_state_webhook_client2::set_channel_connected(bool connected) {
  this->channel_connected = connected;
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

void supla_state_webhook_client2::refresh_token(void) {
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
                            credentials->get_user_short_unique_id());
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

bool supla_state_webhook_client2::perform_post_request(const char *fields,
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

bool supla_state_webhook_client2::send_report(const char *json_string) {
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

bool supla_state_webhook_client2::send_report(cJSON *json) {
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

bool supla_state_webhook_client2::report_with_bool(const char *function,
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

bool supla_state_webhook_client2::report_with_number(const char *function,
                                                     const char *name,
                                                     double number) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      cJSON_AddNumberToObject(state, name, number);
      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client2::value_report(const char *function,
                                               double number) {
  return report_with_number(function, "value", number);
}

bool supla_state_webhook_client2::on_report(const char *function, bool on) {
  return report_with_bool(function, "on", on);
}

bool supla_state_webhook_client2::hi_report(const char *function, bool hi) {
  return report_with_bool(function, "hi", hi);
}

bool supla_state_webhook_client2::power_switch_report(bool on) {
  return on_report("POWERSWITCH", on);
}

bool supla_state_webhook_client2::light_switch_report(bool on) {
  return on_report("LIGHTSWITCH", on);
}

bool supla_state_webhook_client2::staircase_timer_report(bool on) {
  return on_report("STAIRCASETIMER", on);
}

bool supla_state_webhook_client2::temperature_and_humidity_report(
    const char *function, double *temperature, double *humidity) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (temperature) {
        cJSON_AddNumberToObject(state, "temperature", *temperature);
      }

      if (humidity) {
        cJSON_AddNumberToObject(state, "humidity", *humidity);
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

bool supla_state_webhook_client2::temperature_report(double temperature) {
  return temperature_and_humidity_report("THERMOMETER", &temperature, NULL);
}

bool supla_state_webhook_client2::humidity_report(double humidity) {
  return temperature_and_humidity_report("HUMIDITY", NULL, &humidity);
}

bool supla_state_webhook_client2::temperature_and_humidity_report(
    double temperature, double humidity) {
  return temperature_and_humidity_report("HUMIDITYANDTEMPERATURE", &temperature,
                                         &humidity);
}

bool supla_state_webhook_client2::gateway_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_GATEWAY", hi);
}

bool supla_state_webhook_client2::gate_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_GATE", hi);
}

bool supla_state_webhook_client2::garage_door_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_GARAGEDOOR", hi);
}

bool supla_state_webhook_client2::noliquid_sensor_report(bool hi) {
  return hi_report("NOLIQUIDSENSOR", hi);
}

bool supla_state_webhook_client2::door_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_DOOR", hi);
}

bool supla_state_webhook_client2::roller_shutter_opening_sensor_report(
    bool hi) {
  return hi_report("OPENINGSENSOR_ROLLERSHUTTER", hi);
}

bool supla_state_webhook_client2::roof_window_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_ROOFWINDOW", hi);
}

bool supla_state_webhook_client2::window_opening_sensor_report(bool hi) {
  return hi_report("OPENINGSENSOR_WINDOW", hi);
}

bool supla_state_webhook_client2::mail_sensor_report(bool hi) {
  return hi_report("MAILSENSOR", hi);
}

bool supla_state_webhook_client2::shut_report(const char *function, char shut) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      cJSON_AddNumberToObject(state, "shut", shut >= 0 ? shut : 0);
      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddBoolToObject(state, "is_calibrating", shut < 0);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client2::roller_shutter_report(char shut) {
  return shut_report("CONTROLLINGTHEROLLERSHUTTER", shut);
}

bool supla_state_webhook_client2::roof_window_report(char shut) {
  return shut_report("CONTROLLINGTHEROOFWINDOW", shut);
}

bool supla_state_webhook_client2::wind_sensor_report(double value) {
  return value_report("WINDSENSOR", value);
}

bool supla_state_webhook_client2::pressure_sensor_report(double value) {
  return value_report("PRESSURESENSOR", value);
}

bool supla_state_webhook_client2::rain_sensor_report(double value) {
  return value_report("RAINSENSOR", value);
}

bool supla_state_webhook_client2::weight_sensor_report(double value) {
  return value_report("WEIGHTSENSOR", value);
}

bool supla_state_webhook_client2::distance_sensor_report(double distance) {
  return report_with_number("DISTANCESENSOR", "distance", distance);
}

bool supla_state_webhook_client2::depth_sensor_report(double depth) {
  return report_with_number("DEPTHSENSOR", "depth", depth);
}

bool supla_state_webhook_client2::dimmer_and_rgb_report(const char *function,
                                                        int *color,
                                                        char *color_brightness,
                                                        char *brightness,
                                                        char on) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (color) {
        char color_hex[9];
        snprintf(color_hex, sizeof(color_hex), "0x%02X%02X%02X",
                 ((unsigned char *)color)[2], ((unsigned char *)color)[1],
                 ((unsigned char *)color)[0]);
        cJSON_AddStringToObject(state, "color", color_hex);
      }

      if (color_brightness) {
        cJSON_AddNumberToObject(state, "color_brightness", *color_brightness);
      }

      if (brightness) {
        cJSON_AddNumberToObject(state, "brightness", *brightness);
      }

      cJSON_AddBoolToObject(state, "on", on);
      cJSON_AddBoolToObject(state, "connected", channel_connected);
      cJSON_AddItemToObject(root, "state", state);
      return send_report(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client2::dimmer_report(char brightness, char on) {
  return dimmer_and_rgb_report("DIMMER", nullptr, nullptr, &brightness, on);
}

bool supla_state_webhook_client2::dimmer_and_rgb_report(int color,
                                                        char color_brightness,
                                                        char brightness,
                                                        char on) {
  return dimmer_and_rgb_report("DIMMERANDRGBLIGHTING", &color,
                               &color_brightness, &brightness, on);
}

bool supla_state_webhook_client2::rgb_report(int color, char color_brightness,
                                             char on) {
  return dimmer_and_rgb_report("RGBLIGHTING", &color, &color_brightness,
                               nullptr, on);
}

bool supla_state_webhook_client2::electricity_measurement_report(
    supla_channel_electricity_measurement *em) {
  cJSON *root = get_header("ELECTRICITYMETER");
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (channel_connected && em != nullptr) {
        TElectricityMeter_ExtendedValue_V2 em_ev;
        char currency[4];
        em->getMeasurement(&em_ev);
        em->getCurrency(currency);

        if ((em_ev.measured_values & EM_VAR_CURRENT_OVER_65A) &&
            !(em_ev.measured_values & EM_VAR_CURRENT)) {
          em_ev.m[0].current[0] *= 10;
          em_ev.m[0].current[1] *= 10;
          em_ev.m[0].current[2] *= 10;
        }

        cJSON_AddNumberToObject(state, "support", em_ev.measured_values);

        if (strnlen(currency, 4) == 0) {
          cJSON_AddNullToObject(state, "currency");
        } else {
          cJSON_AddStringToObject(state, "currency", currency);
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

bool supla_state_webhook_client2::impulse_counter_measurement_report(
    const char *function, supla_channel_ic_measurement *icm) {
  cJSON *root = get_header(function);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (channel_connected && icm != nullptr) {
        cJSON_AddNumberToObject(state, "totalCost", icm->getTotalCost() * 0.01);
        cJSON_AddNumberToObject(state, "pricePerUnit",
                                icm->getPricePerUnit() * 0.0001);
        cJSON_AddNumberToObject(state, "impulsesPerUnit",
                                icm->getImpulsesPerUnit());
        cJSON_AddNumberToObject(state, "counter", icm->getCounter());
        cJSON_AddNumberToObject(state, "calculatedValue",
                                icm->getCalculatedValue() * 0.001);
        if (strnlen(icm->getCurrency(), 4) == 0) {
          cJSON_AddNullToObject(state, "currency");
        } else {
          cJSON_AddStringToObject(state, "currency", icm->getCurrency());
        }

        if (strnlen(icm->getCustomUnit(), 9) == 0) {
          cJSON_AddNullToObject(state, "unit");
        } else {
          cJSON_AddStringToObject(state, "unit", icm->getCustomUnit());
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

bool supla_state_webhook_client2::
    impulse_counter_electricity_measurement_report(
        supla_channel_ic_measurement *icm) {
  return impulse_counter_measurement_report("IC_ELECTRICITYMETER", icm);
}

bool supla_state_webhook_client2::impulse_counter_gas_measurement_report(
    supla_channel_ic_measurement *icm) {
  return impulse_counter_measurement_report("IC_GASMETER", icm);
}

bool supla_state_webhook_client2::impulse_counter_water_measurement_report(
    supla_channel_ic_measurement *icm) {
  return impulse_counter_measurement_report("IC_WATERMETER", icm);
}

bool supla_state_webhook_client2::impulse_counter_heat_measurement_report(
    supla_channel_ic_measurement *icm) {
  return impulse_counter_measurement_report("IC_HEATMETER", icm);
}

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