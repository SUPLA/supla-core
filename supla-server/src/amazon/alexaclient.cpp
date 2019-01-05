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

#include "amazon/alexa.h"
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include "amazon/alexaclient.h"
#include "http/trivialhttps.h"
#include "json/cJSON.h"
#include "lck.h"
#include "log.h"
#include "tools.h"
#include "user/user.h"
#include "voiceassistantclient.h"

// #define ONLY_LOG_REQUESTS

#if defined(ONLY_LOG_REQUESTS) && !defined(__DEBUG)
#error "Do not use this macro except for the debug profile"
#endif /*ONLY_LOG_REQUESTS*/

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

typedef struct {
  char *str;
  int code;
} _alexa_code_t;

// https://developer.amazon.com/docs/smarthome/send-events-to-the-alexa-event-gateway.html
static const _alexa_code_t alexa_codes[]{
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
static const _alexa_code_t alexa_causes[]{
    {(char *)"APP_INTERACTION", CAUSE_APP_INTERACTION},
    {(char *)"PHYSICAL_INTERACTION", CAUSE_PHYSICAL_INTERACTION},
    {(char *)"PERIODIC_POLL", CAUSE_PERIODIC_POLL},
    {(char *)"RULE_TRIGGER", CAUSE_RULE_TRIGGER},
    {(char *)"VOICE_INTERACTION", CAUSE_VOICE_INTERACTION},
    {NULL, 0},
};

supla_alexa_client::supla_alexa_client(supla_amazon_alexa *alexa)
    : supla_voice_assistant_client(alexa) {}

int supla_alexa_client::getErrorCode(const char *code) {
  int n = 0;
  while (alexa_codes[n].str) {
    if (strncmp(alexa_codes[n].str, code, 50) == 0) {
      return alexa_codes[n].code;
    }
    n++;
  };

  return POST_RESULT_UNKNOWN_ERROR;
}

supla_amazon_alexa *supla_alexa_client::getAlexa(void) {
  return static_cast<supla_amazon_alexa *>(getVoiceAssistant());
}

const char *supla_alexa_client::getErrorString(const int code) {
  int n = 0;
  while (alexa_codes[n].str) {
    if (alexa_codes[n].code == code) {
      return alexa_codes[n].str;
    }
    n++;
  };

  return "UNKNOWN";
}

void supla_alexa_client::refresh_roken(void) {
  if (!getAlexa()->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = getAlexa()->getSetTime();
  getAlexa()->refresh_lock();

  struct timeval current_set_time = getAlexa()->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL
    char host[] = "xatgh8yc1j.execute-api.eu-west-1.amazonaws.com";
    char resource[] = "/default/amazonRefreshTokenBridge";

    getHttps()->setHost(host);
    getHttps()->setResource(resource);
    {
      char *refresh_token = getAlexa()->getRefreshToken();
      if (refresh_token) {
        int rtoken_len = strnlen(refresh_token, ALEXA_TOKEN_MAXSIZE);
        if (rtoken_len != 0) {
          cJSON *root = cJSON_CreateObject();
          cJSON_AddStringToObject(root, "refresh_token", refresh_token);
          char *str = cJSON_PrintUnformatted(root);
          cJSON_Delete(root);

          getHttps()->http_post(NULL, str);
          free(str);
        }

        free(refresh_token);
      }
    }

    if (getHttps()->getResultCode() == 200 && getHttps()->getBody()) {
      cJSON *root = cJSON_Parse(getHttps()->getBody());
      if (root) {
        cJSON *access_token = cJSON_GetObjectItem(root, "access_token");
        cJSON *expires_in = cJSON_GetObjectItem(root, "expires_in");
        cJSON *refresh_token = cJSON_GetObjectItem(root, "refresh_token");

        if (access_token && cJSON_IsString(access_token) && refresh_token &&
            cJSON_IsString(refresh_token)) {
          int ex_in = cJSON_IsNumber(expires_in) ? expires_in->valueint : 0;
          getAlexa()->update(cJSON_GetStringValue(access_token),
                             cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }

    httpsFree();

#endif /*NOSSL*/
  }

  getAlexa()->refresh_unlock();
}

int supla_alexa_client::aeg_post_request(char *data, int *httpResultCode) {
  int result = POST_RESULT_UNKNOWN_ERROR;

  if (httpResultCode) {
    *httpResultCode = 0;
  }

#ifdef NOSSL
  return POST_RESULT_NOSSL;
#else
  {
    char host[30];
    host[0] = 0;
    char *region = getAlexa()->getRegion();

    snprintf(host, 30, "api.%s%samazonalexa.com", region ? region : "",
             region ? "." : "");
    getHttps()->setHost(host);

    if (region) {
      free(region);
    }
  }

  char header[] = "Content-Type: application/json";
  char resource[] = "/v3/events";
  getHttps()->setResource(resource);
  getHttps()->setToken(getAlexa()->getAccessToken(), false);

  if (!getHttps()->http_post(header, data)) {
    return POST_RESULT_REQUEST_ERROR;
  } else {
    if (httpResultCode) {
      *httpResultCode = getHttps()->getResultCode();
    }
    if (getHttps()->getResultCode() != 200 &&
        getHttps()->getResultCode() != 202) {
      if (getHttps()->getBody()) {
        cJSON *root = cJSON_Parse(getHttps()->getBody());
        if (root) {
          cJSON *payload = cJSON_GetObjectItem(root, "payload");
          if (payload) {
            cJSON *code = cJSON_GetObjectItem(payload, "code");
            if (code) {
              result = getErrorCode(cJSON_GetStringValue(code));
            }
          }
          cJSON_Delete(root);
        }
      }
    } else {
      result = POST_RESULT_SUCCESS;
    }
  }

  httpsFree();

#endif /*NOSSL*/

  return result;
}

int supla_alexa_client::aeg_post(char *data) {
  if (!getAlexa()->isAccessTokenExists()) {
    return POST_RESULT_TOKEN_DOES_NOT_EXISTS;
  }

  bool refresh_attempt = false;

  if (getAlexa()->expiresIn() <= 30) {
    refresh_attempt = true;
    refresh_roken();
  }

  int httpResultCode = 0;
  int result = aeg_post_request(data, &httpResultCode);
  if (result == POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION) {
    if (!refresh_attempt) {
      refresh_roken();
      result = aeg_post_request(data, &httpResultCode);
    }
  } else if (result == POST_RESULT_SKILL_DISABLED_EXCEPTION ||
             result == POST_RESULT_SKILL_NOT_FOUND_EXCEPTION) {
    getAlexa()->remove();
    return result;
  }

  if (result != POST_RESULT_SUCCESS) {
    supla_log(LOG_ERR, "Alexa event gateway client error (%i/%i): %s",
              httpResultCode, result, getErrorString(result));
  }

  return result;
}

void *supla_alexa_client::getPowerControllerProperties(bool hi) {
  char now[64];
  st_get_zulu_time(now);

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.PowerController");
    cJSON_AddStringToObject(property, "name", "powerState");
    cJSON_AddStringToObject(property, "value", hi ? "ON" : "OFF");
    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getBrightnessControllerProperties(short brightness) {
  char now[64];
  st_get_zulu_time(now);

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
    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getPercentageControllerProperties(short percentage) {
  char now[64];
  st_get_zulu_time(now);

  if (percentage > 100) {
    percentage = 100;
  } else if (percentage < 0) {
    percentage = 0;
  }

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace",
                            "Alexa.PercentageController");
    cJSON_AddStringToObject(property, "name", "percentage");
    cJSON_AddNumberToObject(property, "value", percentage);
    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getColorControllerProperties(int color,
                                                       short brightness) {
  char now[64];
  st_get_zulu_time(now);

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
    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getContactSensorProperties(bool hi) {
  char now[64];
  st_get_zulu_time(now);

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.ContactSensor");
    cJSON_AddStringToObject(property, "name", "detectionState");
    cJSON_AddStringToObject(property, "value",
                            hi ? "NOT_DETECTED" : "DETECTED");
    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getEndpointHealthProperties(bool ok) {
  char now[64];
  st_get_zulu_time(now);

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.EndpointHealth");
    cJSON_AddStringToObject(property, "name", "connectivity");
    cJSON *value = cJSON_CreateObject();
    if (value) {
      cJSON_AddStringToObject(value, "value", ok ? "OK" : "UNREACHABLE");
      cJSON_AddItemToObject(property, "value", value);
    }

    cJSON_AddStringToObject(property, "timeOfSample", now);
    cJSON_AddNumberToObject(property, "uncertaintyInMilliseconds", 50);
  }

  return property;
}

void *supla_alexa_client::getHeader(const char name[],
                                    const char correlationToken[]) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    char msgId[37];
    msgId[0] = 0;
    st_uuid_v4(msgId);

    cJSON_AddStringToObject(header, "messageId", msgId);
    cJSON_AddStringToObject(header, "namespace", "Alexa");
    cJSON_AddStringToObject(header, "name", name);
    if (correlationToken) {
      cJSON_AddStringToObject(header, "correlationToken", correlationToken);
    }
    cJSON_AddStringToObject(header, "payloadVersion", "3");
  }

  return header;
}

void *supla_alexa_client::getChangeReportHeader(void) {
  return getHeader("ChangeReport", NULL);
}

void *supla_alexa_client::getErrorHeader(const char correlationToken[]) {
  return getHeader("ErrorResponse", correlationToken);
}

void *supla_alexa_client::getResponseHeader(const char correlationToken[]) {
  return getHeader("Response", correlationToken);
}

void *supla_alexa_client::getEndpoint(int channelId, short subChannel) {
  cJSON *endpoint = cJSON_CreateObject();
  if (endpoint) {
    char *token = getAlexa()->getAccessToken();
    if (token) {
      cJSON *scope = cJSON_CreateObject();
      if (scope) {
        cJSON_AddStringToObject(scope, "type", "BearerToken");
        cJSON_AddStringToObject(scope, "token", token);
        cJSON_AddItemToObject(endpoint, "scope", scope);
      }

      free(token);
    }

    char *endpointId = getEndpointId(channelId, subChannel);
    if (endpointId) {
      cJSON_AddStringToObject(endpoint, "endpointId", endpointId);
      free(endpointId);
    }
  }

  return endpoint;
}

void *supla_alexa_client::getChangeReport(int channelId, int causeType,
                                          void *context_properties,
                                          void *change_properties,
                                          short subChannel) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *context = cJSON_CreateObject();
    if (context) {
      if (context_properties) {
        cJSON_AddItemToObject(context, "properties",
                              static_cast<cJSON *>(context_properties));
      }

      cJSON_AddItemToObject(root, "context", context);
    }

    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON_AddItemToObject(root, "event", event);
      cJSON *header = static_cast<cJSON *>(getChangeReportHeader());
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint =
          static_cast<cJSON *>(getEndpoint(channelId, subChannel));
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
              if (alexa_causes[n].code == causeType) {
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

          if (change_properties) {
            cJSON_AddItemToObject(change, "properties",
                                  static_cast<cJSON *>(change_properties));
          }

          cJSON_AddItemToObject(payload, "change", change);
        }

        cJSON_AddItemToObject(event, "payload", payload);
      }
    }
  }

  return root;
}

void *supla_alexa_client::addProps(void *props_array, void *props) {
  if (!props) {
    return props_array;
  }

  if (!props_array) {
    props_array = cJSON_CreateArray();
  }

  if (props_array) {
    cJSON_AddItemToArray(static_cast<cJSON *>(props_array),
                         static_cast<cJSON *>(props));
  }
  return props_array;
}

// https://developer.amazon.com/docs/device-apis/alexa-interface.html#changereport
bool supla_alexa_client::sendChangeReport(int causeType, int channelId,
                                          bool online, void *context_props,
                                          void *change_props,
                                          short subChannel) {
  char *data = NULL;

  if (online) {
    context_props = addProps(context_props, getEndpointHealthProperties(true));
  }

  change_props = addProps(change_props, getEndpointHealthProperties(online));

  cJSON *root = static_cast<cJSON *>(getChangeReport(
      channelId, causeType, context_props, change_props, subChannel));

  if (root) {
#ifdef ONLY_LOG_REQUESTS
    data = cJSON_Print(root);

#else
    data = cJSON_PrintUnformatted(root);
#endif /*ONLY_LOG_REQUESTS*/

    cJSON_Delete(root);
  }
#ifdef ONLY_LOG_REQUESTS
  int result = POST_RESULT_SUCCESS;
#else
  int result = aeg_post(data);
#endif /*ONLY_LOG_REQUESTS*/

  if (data) {
#ifdef ONLY_LOG_REQUESTS
    supla_log(LOG_DEBUG, "%s", data);
#endif /*ONLY_LOG_REQUESTS*/
    free(data);
    data = NULL;
  }

  return result == POST_RESULT_SUCCESS;
}

bool supla_alexa_client::sendPowerChangeReport(int causeType, int channelId,
                                               bool hi, bool online) {
  void *context_props = NULL;
  if (online) {
    context_props = addProps(context_props, getPowerControllerProperties(hi));
  }

  void *change_props = NULL;
  if (online) {
    change_props = addProps(change_props, getPowerControllerProperties(hi));
  }

  return POST_RESULT_SUCCESS == sendChangeReport(causeType, channelId, online,
                                                 context_props, change_props);
}

bool supla_alexa_client::sendContactChangeReport(int causeType, int channelId,
                                                 bool hi, bool online) {
  void *context_props = NULL;
  if (online) {
    context_props = addProps(context_props, getContactSensorProperties(hi));
  }

  void *change_props = NULL;
  if (online) {
    change_props = addProps(change_props, getContactSensorProperties(hi));
  }

  return POST_RESULT_SUCCESS == sendChangeReport(causeType, channelId, online,
                                                 context_props, change_props);
}

bool supla_alexa_client::sendBrightnessChangeReport(int causeType,
                                                    int channelId,
                                                    short brightness,
                                                    bool online,
                                                    short subChannel) {
  void *context_props = NULL;
  if (online) {
    context_props =
        addProps(context_props, getBrightnessControllerProperties(brightness));
    context_props =
        addProps(context_props, getPowerControllerProperties(brightness > 0));
  }

  void *change_props = NULL;
  if (online) {
    change_props =
        addProps(change_props, getBrightnessControllerProperties(brightness));
    change_props =
        addProps(change_props, getPowerControllerProperties(brightness > 0));
  }

  return POST_RESULT_SUCCESS == sendChangeReport(causeType, channelId, online,
                                                 context_props, change_props,
                                                 subChannel);
}

bool supla_alexa_client::sendColorChangeReport(int causeType, int channelId,
                                               int color, short colorBrightness,
                                               bool online, short subChannel) {
  if (!online) {
    colorBrightness = 0;
  }

  void *context_props = NULL;
  if (online) {
    context_props = addProps(
        context_props, getColorControllerProperties(color, colorBrightness));
    context_props = addProps(
        context_props, getBrightnessControllerProperties(colorBrightness));
    context_props = addProps(context_props,
                             getPowerControllerProperties(colorBrightness > 0));
  }

  void *change_props = NULL;
  if (online) {
    change_props = addProps(
        change_props, getColorControllerProperties(color, colorBrightness));
    change_props = addProps(change_props,
                            getBrightnessControllerProperties(colorBrightness));
    change_props = addProps(change_props,
                            getPowerControllerProperties(colorBrightness > 0));
  }

  return POST_RESULT_SUCCESS == sendChangeReport(causeType, channelId, online,
                                                 context_props, change_props,
                                                 subChannel);
}

bool supla_alexa_client::sendPercentageChangeReport(int causeType,
                                                    int channelId,
                                                    short percentage,
                                                    bool online) {
  void *context_props = NULL;
  if (online) {
    context_props =
        addProps(context_props, getPercentageControllerProperties(percentage));
  }

  void *change_props = NULL;
  if (online) {
    change_props =
        addProps(change_props, getPercentageControllerProperties(percentage));
  }

  return POST_RESULT_SUCCESS == sendChangeReport(causeType, channelId, online,
                                                 context_props, change_props,
                                                 0);
}

void *supla_alexa_client::getEndpointUnrechableErrorResponse(
    const char correlationToken[], int channelId, short subChannel) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON *header = static_cast<cJSON *>(getErrorHeader(correlationToken));
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint =
          static_cast<cJSON *>(getEndpoint(channelId, subChannel));
      if (endpoint) {
        cJSON_AddItemToObject(event, "endpoint", endpoint);
      }

      cJSON *payload = cJSON_CreateObject();
      if (payload) {
        char message[80];
        snprintf(message, sizeof(message),
                 "Unable to reach channel ID%i because it appears to be "
                 "offline.",
                 channelId);
        cJSON_AddStringToObject(payload, "message", message);
        cJSON_AddStringToObject(payload, "type", "ENDPOINT_UNREACHABLE");

        cJSON_AddItemToObject(event, "payload", payload);
      }

      cJSON_AddItemToObject(root, "event", event);
    }
  }
  return root;
}

void *supla_alexa_client::getResponse(const char correlationToken[],
                                      int channelId, void *props,
                                      short subChannel) {
  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *context = cJSON_CreateObject();
    if (context) {
      if (props) {
        cJSON_AddItemToObject(context, "properties",
                              static_cast<cJSON *>(props));
      }
      cJSON_AddItemToObject(root, "context", context);
    }

    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON *header = static_cast<cJSON *>(getResponseHeader(correlationToken));
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint =
          static_cast<cJSON *>(getEndpoint(channelId, subChannel));
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

bool supla_alexa_client::sendResponse(const char correlationToken[],
                                      int channelId, bool online, void *props,
                                      short subChannel) {
  char *data = NULL;
  cJSON *root = NULL;

  if (online) {
    root = static_cast<cJSON *>(
        getResponse(correlationToken, channelId, props, subChannel));
  } else {
    root = static_cast<cJSON *>(getEndpointUnrechableErrorResponse(
        correlationToken, channelId, subChannel));
  }

  if (root) {
#ifdef ONLY_LOG_REQUESTS
    data = cJSON_Print(root);
#else
    data = cJSON_PrintUnformatted(root);
#endif /*ONLY_LOG_REQUESTS*/
    cJSON_Delete(root);
  }
#ifdef ONLY_LOG_REQUESTS
  int result = POST_RESULT_SUCCESS;  // aeg_post(data);
#else
  int result = aeg_post(data);
#endif /*ONLY_LOG_REQUESTS*/

  if (data) {
#ifdef ONLY_LOG_REQUESTS
    supla_log(LOG_DEBUG, "%s", data);
#endif /*ONLY_LOG_REQUESTS*/
    free(data);
    data = NULL;
  }

  return result == POST_RESULT_SUCCESS;
}

bool supla_alexa_client::powerControllerSendResponse(
    const char correlationToken[], int channelId, bool hi, bool online) {
  void *props = NULL;
  if (online) {
    props = addProps(props, getPowerControllerProperties(hi));
  }

  return POST_RESULT_SUCCESS ==
         sendResponse(correlationToken, channelId, online, props, 0);
}

bool supla_alexa_client::brightnessControllerSendResponse(
    const char correlationToken[], int channelId, short brightness, bool online,
    short subChannel) {
  void *props = NULL;
  if (online) {
    props = addProps(props, getBrightnessControllerProperties(brightness));
    props = addProps(props, getPowerControllerProperties(brightness > 0));
  }

  return POST_RESULT_SUCCESS ==
         sendResponse(correlationToken, channelId, online, props, subChannel);
}

bool supla_alexa_client::colorControllerSendResponse(
    const char correlationToken[], int channelId, int color,
    short colorBrightness, bool online, short subChannel) {
  void *props = NULL;
  if (online) {
    props =
        addProps(props, getColorControllerProperties(color, colorBrightness));
    props = addProps(props, getBrightnessControllerProperties(colorBrightness));
    props = addProps(props, getPowerControllerProperties(colorBrightness > 0));
  }

  return POST_RESULT_SUCCESS ==
         sendResponse(correlationToken, channelId, online, props, subChannel);
}

bool supla_alexa_client::percentageControllerSendResponse(
    const char correlationToken[], int channelId, short percentage,
    bool online) {
  void *props = NULL;
  if (online) {
    props = addProps(props, getPercentageControllerProperties(percentage));
  }

  return POST_RESULT_SUCCESS ==
         sendResponse(correlationToken, channelId, online, props, 0);
}
