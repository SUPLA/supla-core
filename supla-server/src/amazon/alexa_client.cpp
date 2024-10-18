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

#include "alexa_client.h"

#include <string.h>

#include "alexa_access_token_refresh_agent.h"
#include "device/value/channel_binary_sensor_value.h"
#include "device/value/channel_fb_value.h"
#include "device/value/channel_hvac_value_with_temphum.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "log.h"
#include "tools.h"

using std::string;

#define POST_RESULT_SUCCESS 1
#define POST_RESULT_UNKNOWN_ERROR 0
#define POST_RESULT_NOSSL -100
#define POST_RESULT_TOKEN_DOES_NOT_EXISTS -200
#define POST_RESULT_REQUEST_ERROR -300

#define POST_RESULT_INVALID_REQUEST_EXCEPTION -1
#define POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION -2
#define POST_RESULT_SKILL_DISABLED_EXCEPTION -3
#define POST_RESULT_INSUFFICIENT_PERMISSION_EXCEPTION -4
#define POST_RESULT_SKILL_NOT_FOUND_EXCEPTION 1005
#define POST_RESULT_REQUEST_ENTITY_TOO_LARGE_EXCEPTION -5
#define POST_RESULT_THROTTLING_EXCEPTION -6
#define POST_RESULT_INTERNAL_SERVICE_EXCEPTION -7
#define POST_RESULT_SERVICE_UNAVAILABLE_EXCEPTION -8

// https://developer.amazon.com/docs/smarthome/send-events-to-the-alexa-event-gateway.html
// static
const supla_alexa_client::_alexa_code_t supla_alexa_client::alexa_codes[]{
    {(char *)"SUCCESS", POST_RESULT_SUCCESS},
    {(char *)"UNKNOWN_ERROR", POST_RESULT_UNKNOWN_ERROR},
    {(char *)"NOSSL", POST_RESULT_NOSSL},
    {(char *)"TOKEN_DOES_NOT_EXISTS", POST_RESULT_TOKEN_DOES_NOT_EXISTS},
    {(char *)"REQUEST_ERROR", POST_RESULT_REQUEST_ERROR},
    {(char *)"INVALID_REQUEST_EXCEPTION",
     POST_RESULT_INVALID_REQUEST_EXCEPTION},
    {(char *)"INVALID_ACCESS_TOKEN_EXCEPTION",
     POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION},
    {(char *)"SKILL_DISABLED_EXCEPTION", POST_RESULT_SKILL_DISABLED_EXCEPTION},
    {(char *)"INSUFFICIENT_PERMISSION_EXCEPTION",
     POST_RESULT_INSUFFICIENT_PERMISSION_EXCEPTION},
    {(char *)"SKILL_NOT_FOUND_EXCEPTION",
     POST_RESULT_SKILL_NOT_FOUND_EXCEPTION},
    {(char *)"REQUEST_ENTITY_TOO_LARGE_EXCEPTION",
     POST_RESULT_REQUEST_ENTITY_TOO_LARGE_EXCEPTION},
    {(char *)"THROTTLING_EXCEPTION", POST_RESULT_THROTTLING_EXCEPTION},
    {(char *)"INTERNAL_SERVICE_EXCEPTION",
     POST_RESULT_INTERNAL_SERVICE_EXCEPTION},
    {(char *)"SERVICE_UNAVAILABLE_EXCEPTION",
     POST_RESULT_SERVICE_UNAVAILABLE_EXCEPTION},
    {NULL, 0},
};

// https://developer.amazon.com/docs/smarthome/state-reporting-for-a-smart-home-skill.html#cause-object
// static
const supla_alexa_client::_alexa_code_t supla_alexa_client::alexa_causes[]{
    {(char *)"APP_INTERACTION", CAUSE_APP_INTERACTION},
    {(char *)"PHYSICAL_INTERACTION", CAUSE_PHYSICAL_INTERACTION},
    {(char *)"PERIODIC_POLL", CAUSE_PERIODIC_POLL},
    {(char *)"RULE_TRIGGER", CAUSE_RULE_TRIGGER},
    {(char *)"VOICE_INTERACTION", CAUSE_VOICE_INTERACTION},
    {NULL, 0},
};

supla_alexa_client::supla_alexa_client(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_amazon_alexa_credentials *credentials, const string &zulu_time,
    const string &message_id, const string &correlation_token)
    : supla_voice_assistant_client(channel_id, curl_adapter, credentials) {
  cause_type = CAUSE_APP_INTERACTION;
  props_arr = nullptr;
  endpoints = nullptr;
  this->zulu_time = zulu_time;
  this->message_id = message_id;
  this->correlation_token = correlation_token;

  if (this->zulu_time.empty()) {
    char now[64] = {};
    st_get_zulu_time(now);
    this->zulu_time = now;
  }

  if (this->message_id.empty()) {
    char msgid[37] = {};
    st_uuid_v4(msgid);
    this->message_id = msgid;
  }
}

supla_alexa_client::~supla_alexa_client(void) {
  if (props_arr) {
    cJSON_Delete(props_arr);
  }

  if (endpoints) {
    cJSON_Delete(endpoints);
  }
}

const char *supla_alexa_client::get_error_code_str(const int code) {
  int n = 0;
  while (alexa_codes[n].str) {
    if (alexa_codes[n].code == code) {
      return alexa_codes[n].str;
    }
    n++;
  }

  return "UNKNOWN";
}

int supla_alexa_client::get_error_code(const char *code) {
  int n = 0;
  while (alexa_codes[n].str) {
    if (strncmp(alexa_codes[n].str, code, 50) == 0) {
      return alexa_codes[n].code;
    }
    n++;
  }

  return POST_RESULT_UNKNOWN_ERROR;
}

supla_amazon_alexa_credentials *supla_alexa_client::get_alexa_credentials(
    void) {
  return static_cast<supla_amazon_alexa_credentials *>(get_credentials());
}

void supla_alexa_client::set_cause_type(int cause_type) {
  this->cause_type = cause_type;
}

void supla_alexa_client::set_cause_type(const supla_caller &caller) {
  switch (caller.get_type()) {
    case ctAmazonAlexa:
      set_cause_type(CAUSE_VOICE_INTERACTION);
      break;

    case ctClient:
    case ctIPC:
    case ctMQTT:
    case ctGoogleHome:
    case ctScene:
      set_cause_type(CAUSE_APP_INTERACTION);
      break;
    default:
      set_cause_type(CAUSE_PHYSICAL_INTERACTION);
      break;
  }
}

cJSON *supla_alexa_client::add_props(cJSON *props_arr, cJSON *props) {
  if (!props) {
    return props_arr;
  }

  if (!props_arr) {
    props_arr = cJSON_CreateArray();
  }

  if (props_arr) {
    cJSON_AddItemToArray(props_arr, props);
  }

  return props_arr;
}

void supla_alexa_client::add_props(cJSON *props) {
  props_arr = add_props(props_arr, props);
}

cJSON *supla_alexa_client::get_power_controller_properties(bool hi) {
  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.PowerController");
    cJSON_AddStringToObject(property, "name", "powerState");
    cJSON_AddStringToObject(property, "value", hi ? "ON" : "OFF");
    cJSON_AddStringToObject(property, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

cJSON *supla_alexa_client::get_brightness_controller_properties(
    short brightness) {
  if (brightness > 100) {
    brightness = 100;
  } else if (brightness < 0) {
    brightness = 0;
  }

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace",
                            "Alexa.BrightnessController");
    cJSON_AddStringToObject(property, "name", "brightness");
    cJSON_AddNumberToObject(property, "value", brightness);
    cJSON_AddStringToObject(property, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

cJSON *supla_alexa_client::get_color_controller_properties(int color,
                                                           short brightness) {
  if (brightness > 100) {
    brightness = 100;
  } else if (brightness < 0) {
    brightness = 0;
  }

  _color_hsv_t hsv = st_rgb2hsv(color);

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.ColorController");
    cJSON_AddStringToObject(property, "name", "color");
    cJSON *value = cJSON_CreateObject();
    if (value) {
      cJSON_AddNumberToObject(value, "hue", hsv.h);
      cJSON_AddNumberToObject(value, "saturation", hsv.s);
      cJSON_AddNumberToObject(value, "brightness", brightness / 100.00);
      cJSON_AddItemToObject(property, "value", value);
    }
    cJSON_AddStringToObject(property, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

cJSON *supla_alexa_client::get_range_controller_properties(short range) {
  if (range > 100) {
    range = 100;
  } else if (range < 0) {
    range = 0;
  }

  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.RangeController");
    cJSON_AddStringToObject(props, "instance", "Blind.Lift");
    cJSON_AddStringToObject(props, "name", "rangeValue");
    cJSON_AddNumberToObject(props, "value", range);
    cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
  }

  return props;
}

cJSON *supla_alexa_client::get_percentage_controller_properties(
    short percentage) {
  if (percentage > 100) {
    percentage = 100;
  } else if (percentage < 0) {
    percentage = 0;
  }

  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.PercentageController");
    cJSON_AddStringToObject(props, "name", "percentage");
    cJSON_AddNumberToObject(props, "value", percentage);
    cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
  }

  return props;
}

cJSON *supla_alexa_client::get_contact_sensor_properties(bool hi) {
  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.ContactSensor");
    cJSON_AddStringToObject(props, "name", "detectionState");
    cJSON_AddStringToObject(props, "value", hi ? "NOT_DETECTED" : "DETECTED");
    cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
  }

  return props;
}

cJSON *supla_alexa_client::get_thermostat_mode_properties(string mode) {
  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.ThermostatController");
    cJSON_AddStringToObject(props, "name", "thermostatMode");
    cJSON_AddStringToObject(props, "value", mode.c_str());
    cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
  }

  return props;
}

cJSON *supla_alexa_client::get_thermostat_setpoint_properties(
    string name, double temperature) {
  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.ThermostatController");
    cJSON_AddStringToObject(props, "name", name.c_str());

    cJSON *value = cJSON_CreateObject();
    if (value) {
      cJSON_AddNumberToObject(value, "value", temperature);
      cJSON_AddStringToObject(value, "scale", "CELSIUS");
      cJSON_AddItemToObject(props, "value", value);
      cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
      cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
    }
  }

  return props;
}

cJSON *supla_alexa_client::get_temperature_sensor_properties(
    double temperature) {
  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.TemperatureSensor");
    cJSON_AddStringToObject(props, "name", "temperature");

    cJSON *value = cJSON_CreateObject();
    if (value) {
      cJSON_AddNumberToObject(value, "value", temperature);
      cJSON_AddStringToObject(value, "scale", "CELSIUS");
      cJSON_AddItemToObject(props, "value", value);
      cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
      cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
    }
  }

  return props;
}

cJSON *supla_alexa_client::get_endpoint_health_properties(bool ok) {
  cJSON *props = cJSON_CreateObject();
  if (props) {
    cJSON_AddStringToObject(props, "namespace", "Alexa.EndpointHealth");
    cJSON_AddStringToObject(props, "name", "connectivity");
    cJSON *value = cJSON_CreateObject();
    if (value) {
      cJSON_AddStringToObject(value, "value", ok ? "OK" : "UNREACHABLE");
      cJSON_AddItemToObject(props, "value", value);
    }

    cJSON_AddStringToObject(props, "timeOfSample", zulu_time.c_str());
    cJSON_AddNumberToObject(props, "uncertaintyInMilliseconds", 50);
  }

  return props;
}

cJSON *supla_alexa_client::get_header(const char ns[], const char name[],
                                      bool use_correlation_token) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    cJSON_AddStringToObject(header, "messageId", message_id.c_str());
    cJSON_AddStringToObject(header, "namespace", ns);
    cJSON_AddStringToObject(header, "name", name);
    if (use_correlation_token && !correlation_token.empty()) {
      cJSON_AddStringToObject(header, "correlationToken",
                              correlation_token.c_str());
    }
    cJSON_AddStringToObject(header, "payloadVersion", "3");
  }

  return header;
}

cJSON *supla_alexa_client::get_header(const char name[],
                                      bool use_correlation_token) {
  return get_header("Alexa", name, use_correlation_token);
}

cJSON *supla_alexa_client::get_endpoint(void) {
  cJSON *endpoint = cJSON_CreateObject();
  if (endpoint) {
    cJSON *scope = cJSON_CreateObject();
    if (scope) {
      cJSON_AddStringToObject(scope, "type", "BearerToken");
      cJSON_AddStringToObject(scope, "token",
                              get_credentials()->get_access_token().c_str());
      cJSON_AddItemToObject(endpoint, "scope", scope);
    }

    cJSON_AddStringToObject(endpoint, "endpointId", get_endpoint_id().c_str());
  }

  return endpoint;
}

cJSON *supla_alexa_client::get_response(void) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *context = cJSON_CreateObject();
    if (context) {
      if (props_arr) {
        cJSON_AddItemToObject(context, "properties", props_arr);
        props_arr = nullptr;
      }
      cJSON_AddItemToObject(root, "context", context);
    }

    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON *header = get_header("Response", true);
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint = get_endpoint();
      if (endpoint) {
        cJSON_AddItemToObject(event, "endpoint", endpoint);
      }
      cJSON *payload = cJSON_CreateObject();
      if (payload) {
        cJSON_AddItemToObject(event, "payload", payload);
      }
      cJSON_AddItemToObject(root, "event", event);
    }
  }
  return root;
}

cJSON *supla_alexa_client::get_change_report(void) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *context = cJSON_CreateObject();
    if (context) {
      cJSON *context_properties = cJSON_Duplicate(props_arr, true);
      if (context_properties) {
        if (is_channel_connected()) {
          context_properties = add_props(context_properties,
                                         get_endpoint_health_properties(true));
        }

        cJSON_AddItemToObject(context, "properties",
                              static_cast<cJSON *>(context_properties));
      }

      cJSON_AddItemToObject(root, "context", context);
    }

    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON_AddItemToObject(root, "event", event);
      cJSON *header = get_header("ChangeReport", false);
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint = get_endpoint();
      if (endpoint) {
        cJSON_AddItemToObject(event, "endpoint", endpoint);
      }

      cJSON *payload = cJSON_CreateObject();
      if (payload) {
        cJSON *change = cJSON_CreateObject();
        if (change) {
          cJSON *cause = cJSON_CreateObject();
          if (cause) {
            char *cause_str = NULL;
            int n = 0;
            while (alexa_causes[n].str) {
              if (alexa_causes[n].code == cause_type) {
                cause_str = alexa_causes[n].str;
                break;
              }
              n++;
            }
            if (cause_str) {
              cJSON_AddStringToObject(cause, "type", cause_str);
              cJSON_AddItemToObject(change, "cause", cause);
            }
          }

          add_props(get_endpoint_health_properties(is_channel_connected()));

          if (props_arr) {
            cJSON_AddItemToObject(change, "properties", props_arr);
            props_arr = nullptr;
          }

          cJSON_AddItemToObject(payload, "change", change);
        }

        cJSON_AddItemToObject(event, "payload", payload);
      }
    }
  }

  return root;
}

cJSON *supla_alexa_client::get_delete_report(void) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *event = cJSON_AddObjectToObject(root, "event");
    if (event) {
      cJSON *header = get_header("Alexa.Discovery", "DeleteReport", false);
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *payload = cJSON_AddObjectToObject(event, "payload");
      if (payload) {
        cJSON *scope = cJSON_AddObjectToObject(payload, "scope");
        if (scope) {
          cJSON_AddStringToObject(scope, "type", "BearerToken");
          cJSON_AddStringToObject(
              scope, "token", get_credentials()->get_access_token().c_str());
        }

        if (!endpoints) {
          endpoints = cJSON_CreateArray();
        }

        if (endpoints) {
          cJSON_AddItemToObject(payload, "endpoints", endpoints);
          endpoints = nullptr;
        }
      }
    }
  }
  return root;
}

cJSON *supla_alexa_client::get_unrechable_error_response(void) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON *header = get_header("ErrorResponse", true);
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint = get_endpoint();
      if (endpoint) {
        cJSON_AddItemToObject(event, "endpoint", endpoint);
      }

      cJSON *payload = cJSON_CreateObject();
      if (payload) {
        char message[80] = {};
        snprintf(message, sizeof(message),
                 "Unable to reach channel ID%i because it appears to be "
                 "offline.",
                 get_channel_id());
        cJSON_AddStringToObject(payload, "message", message);
        cJSON_AddStringToObject(payload, "type", "ENDPOINT_UNREACHABLE");

        cJSON_AddItemToObject(event, "payload", payload);
      }

      cJSON_AddItemToObject(root, "event", event);
    }
  }
  return root;
}

int supla_alexa_client::perform_post_request(const char *data,
                                             int *http_result_code,
                                             string *error_description) {
  int result = POST_RESULT_UNKNOWN_ERROR;

  if (error_description) {
    error_description->clear();
  }

  if (http_result_code) {
    *http_result_code = 0;
  }

  string url = "https://api.";

  {
    string region = get_alexa_credentials()->get_region();
    if (!region.empty()) {
      url.append(region);
      url.append(".");
    }
    url.append("amazonalexa.com/v3/events");
  }

  if (!get_credentials()->is_access_token_exists()) {
    return POST_RESULT_TOKEN_DOES_NOT_EXISTS;
  }

  string token = "Authorization: Bearer ";
  token.append(get_credentials()->get_access_token());

  string request_result;

  get_curl_adapter()->reset();
  get_curl_adapter()->set_opt_write_data(&request_result);
  get_curl_adapter()->append_header("Content-Type: application/json");
  get_curl_adapter()->append_header(token.c_str());
  get_curl_adapter()->set_opt_post_fields(data);
  get_curl_adapter()->set_opt_url(url.c_str());

  if (!get_curl_adapter()->perform()) {
    result = POST_RESULT_REQUEST_ERROR;
  } else {
    if (http_result_code) {
      *http_result_code = get_curl_adapter()->get_response_code();
    }
    if (get_curl_adapter()->get_response_code() != 200 &&
        get_curl_adapter()->get_response_code() != 202) {
      if (!request_result.empty()) {
        cJSON *root = cJSON_Parse(request_result.c_str());
        if (root) {
          cJSON *payload = cJSON_GetObjectItem(root, "payload");
          if (payload) {
            cJSON *code = cJSON_GetObjectItem(payload, "code");
            if (code) {
              result = get_error_code(cJSON_GetStringValue(code));
            }

            if (error_description) {
              cJSON *description = cJSON_GetObjectItem(payload, "description");
              if (description && cJSON_IsString(description)) {
                *error_description = cJSON_GetStringValue(description);
              }
            }
          }
          cJSON_Delete(root);
        }
      }
    } else {
      result = POST_RESULT_SUCCESS;
    }
  }

  return result;
}

void supla_alexa_client::refresh_token(void) {
  if (!get_credentials()->is_refresh_token_exists()) {
    return;
  }

  struct timeval last_set_time = get_credentials()->get_set_time();
  get_credentials()->refresh_lock();

  struct timeval current_set_time = get_credentials()->get_set_time();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
    string request_result;

    get_curl_adapter()->reset();
    get_curl_adapter()->set_opt_write_data(&request_result);
    get_curl_adapter()->set_opt_url(
        "https://xatgh8yc1j.execute-api.eu-west-1.amazonaws.com/default/"
        "amazonRefreshTokenBridge");

    string refresh_token = get_credentials()->get_refresh_token();
    if (!refresh_token.empty()) {
      cJSON *root = cJSON_CreateObject();
      cJSON_AddStringToObject(root, "refresh_token", refresh_token.c_str());
      char *data = cJSON_PrintUnformatted(root);
      cJSON_Delete(root);

      get_curl_adapter()->set_opt_post_fields(data);
      get_curl_adapter()->perform();

      free(data);
    }

    if (get_curl_adapter()->get_response_code() == 200 &&
        !request_result.empty()) {
      cJSON *root = cJSON_Parse(request_result.c_str());
      if (root) {
        cJSON *access_token = cJSON_GetObjectItem(root, "access_token");
        cJSON *expires_in = cJSON_GetObjectItem(root, "expires_in");
        cJSON *refresh_token = cJSON_GetObjectItem(root, "refresh_token");

        if (access_token && cJSON_IsString(access_token) && refresh_token &&
            cJSON_IsString(refresh_token)) {
          int ex_in = cJSON_IsNumber(expires_in) ? expires_in->valueint : 0;

          get_alexa_credentials()->update(cJSON_GetStringValue(access_token),
                                          cJSON_GetStringValue(refresh_token),
                                          ex_in);
        }

        cJSON_Delete(root);
      }
    }
  }

  get_credentials()->refresh_unlock();
}

int supla_alexa_client::perform_post_request(const char *data) {
  if (!get_credentials()->is_access_token_exists()) {
    return POST_RESULT_TOKEN_DOES_NOT_EXISTS;
  }

  int http_result_code = 0;
  string error_description;
  int result =
      perform_post_request(data, &http_result_code, &error_description);

  // https://developer.amazon.com/en-US/docs/alexa/smarthome/send-events.html
  // 401 Unauthorized == INVALID_ACCESS_TOKEN_EXCEPTION
  // 403 Forbidden == SKILL_NEVER_ENABLED_EXCEPTION
  // 404 Not Found == SKILL_NOT_FOUND_EXCEPTION

  if (result == POST_RESULT_SKILL_DISABLED_EXCEPTION) {
    get_alexa_credentials()->remove();
    return result;
  }

  if (result != POST_RESULT_SUCCESS) {
    supla_log(LOG_ERR,
              "Alexa event gateway client error (UserID: %i, httpCode: %i, "
              "Result: %i): %s \"%s\"",
              get_credentials()->get_user_id(), http_result_code, result,
              get_error_code_str(result), error_description.c_str());
  }

  return result;
}

void supla_alexa_client::add_power_controller_properties(void) {
  if (is_channel_connected()) {
    supla_channel_onoff_value *onoff_val =
        dynamic_cast<supla_channel_onoff_value *>(get_channel_value());
    if (onoff_val) {
      add_props(get_power_controller_properties(onoff_val->is_on()));
    }
  }
}

void supla_alexa_client::add_brightness_controller_properties(void) {
  if (is_channel_connected()) {
    supla_channel_rgbw_value *rgbw_val =
        dynamic_cast<supla_channel_rgbw_value *>(get_channel_value());
    if (rgbw_val) {
      add_props(
          get_brightness_controller_properties(rgbw_val->get_brightness()));
      add_props(
          get_power_controller_properties(rgbw_val->get_brightness() > 0));
    }
  }
}

void supla_alexa_client::add_color_controller_properties(void) {
  if (is_channel_connected()) {
    supla_channel_rgbw_value *rgbw_val =
        dynamic_cast<supla_channel_rgbw_value *>(get_channel_value());
    if (rgbw_val) {
      add_props(get_color_controller_properties(
          rgbw_val->get_color(), rgbw_val->get_color_brightness()));
      add_props(get_brightness_controller_properties(
          rgbw_val->get_color_brightness()));
      add_props(get_power_controller_properties(
          rgbw_val->get_color_brightness() > 0));
    }
  }
}

void supla_alexa_client::add_range_controller(void) {
  if (is_channel_connected()) {
    supla_channel_rs_value *rs_val =
        dynamic_cast<supla_channel_rs_value *>(get_channel_value());

    supla_channel_fb_value *fb_val = nullptr;

    if (!rs_val) {
      fb_val = dynamic_cast<supla_channel_fb_value *>(get_channel_value());
    }

    if (rs_val || fb_val) {
      add_props(get_range_controller_properties(
          rs_val ? rs_val->get_rs_value()->position : fb_val->get_position()));
    }
  }
}

void supla_alexa_client::add_percentage_controller(void) {
  if (is_channel_connected()) {
    supla_channel_rs_value *rs_val =
        dynamic_cast<supla_channel_rs_value *>(get_channel_value());

    supla_channel_fb_value *fb_val = nullptr;

    if (!rs_val) {
      fb_val = dynamic_cast<supla_channel_fb_value *>(get_channel_value());
    }

    if (rs_val || fb_val) {
      add_props(get_percentage_controller_properties(
          rs_val ? rs_val->get_rs_value()->position : fb_val->get_position()));
    }
  }
}

void supla_alexa_client::add_contact_sensor(void) {
  if (is_channel_connected()) {
    supla_channel_binary_sensor_value *bs_val =
        dynamic_cast<supla_channel_binary_sensor_value *>(get_channel_value());
    if (bs_val) {
      add_props(get_contact_sensor_properties(bs_val->is_hi()));
    }
  }
}

void supla_alexa_client::add_thermostat_controller(void) {
  if (is_channel_connected()) {
    supla_channel_hvac_value_with_temphum *hvac_val =
        dynamic_cast<supla_channel_hvac_value_with_temphum *>(
            get_channel_value());
    if (hvac_val) {
      add_props(get_thermostat_mode_properties(hvac_val->get_alexa_mode()));
      if (hvac_val->get_mode() == SUPLA_HVAC_MODE_HEAT_COOL) {
        add_props(get_thermostat_setpoint_properties(
            "lowerSetpoint", hvac_val->get_setpoint_temperature_heat_dbl()));
        add_props(get_thermostat_setpoint_properties(
            "upperSetpoint", hvac_val->get_setpoint_temperature_cool_dbl()));
      } else {
        add_props(get_thermostat_setpoint_properties(
            "targetSetpoint", hvac_val->get_setpoint_temperature_dbl()));
      }
      add_props(
          get_temperature_sensor_properties(hvac_val->get_temperature_dbl()));
    }
  }
}

void supla_alexa_client::add_deleted_endpoint(int id, bool scene) {
  cJSON *deleted = cJSON_CreateObject();
  cJSON_AddStringToObject(deleted, "endpointId",
                          get_endpoint_id(id, 0, scene).c_str());

  if (!endpoints) {
    endpoints = cJSON_CreateArray();
  }

  cJSON_AddItemToArray(endpoints, deleted);
}

bool supla_alexa_client::send_response(void) {
  char *data = NULL;
  cJSON *root = NULL;

  if (is_channel_connected()) {
    root = get_response();
  } else {
    root = get_unrechable_error_response();
  }

  if (root) {
    data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
  }

  int result = perform_post_request(data);

  if (data) {
    free(data);
    data = NULL;
  }

  return result == POST_RESULT_SUCCESS;
}

bool supla_alexa_client::send_report(cJSON *root) {
  char *data = nullptr;

  if (root) {
    data = cJSON_PrintUnformatted(root);

    cJSON_Delete(root);
  }

  int result = perform_post_request(data);

  if (data) {
    free(data);
  }

  return result == POST_RESULT_SUCCESS;
}

// https://developer.amazon.com/docs/device-apis/alexa-interface.html#changereport
bool supla_alexa_client::send_change_report(void) {
  return send_report(get_change_report());
}

// https://developer.amazon.com/en-US/docs/alexa/device-apis/alexa-discovery.html#deletereport-event
bool supla_alexa_client::send_delete_report(void) {
  return send_report(get_delete_report());
}

bool supla_alexa_client::send_payload(std::string payload) {
  return perform_post_request(payload.c_str()) == POST_RESULT_SUCCESS;
}
