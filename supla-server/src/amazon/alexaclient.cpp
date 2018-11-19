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

#include <amazon/alexa.h>
#include <amazon/alexaclient.h>
#include <http/trivialhttps.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include "json/cJSON.h"
#include "log.h"
#include "tools.h"

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

/*
 * Indicates that the event was caused by a customer interaction with an
 * application. For example, a customer switches on a light or locks a door
 * using the Alexa app or an app provided by a device vendor.
 */
#define CAUSE_APP_INTERACTION 0

/*
 * Indicates that the event was caused by a physical interaction with an
 * endpoint. For example, manually switching on a light or manually locking a
 * door lock.
 */
#define CAUSE_PHYSICAL_INTERACTION 1

/*
 * Indicates that the event was caused by the periodic poll of an endpoint,
 * which found a change in value. For example, you might poll a temperature
 * sensor every hour and send the updated temperature to Alexa.
 */
#define CAUSE_PERIODIC_POLL 2

/*
 * Indicates that the event was caused by the application of a device rule. For
 * example, a customer configures a rule to switch on a light if a motion sensor
 * detects motion. In this case, Alexa receives an event from the motion sensor,
 * and another event from the light to indicate that its state change was caused
 * by the rule.
 */
#define CAUSE_RULE_TRIGGER 3

/*
 * Indicates that the event was caused by a voice interaction. For example, a
 * user speaking to their Echo device.
 */
#define CAUSE_VOICE_INTERACTION 4

typedef struct {
  char *str;
  int code;
} _alexa_code_t;

// https://developer.amazon.com/docs/smarthome/send-events-to-the-alexa-event-gateway.html
static const _alexa_code_t alexa_codes[]{
    {(char *)"SUCCESS", POST_RESULT_UNKNOWN_ERROR},
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

supla_alexa_client::supla_alexa_client(supla_amazon_alexa *alexa) {
  this->alexa = alexa;
}

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

supla_alexa_client::~supla_alexa_client() {}

void supla_alexa_client::refresh_roken(void) {
  if (!alexa->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = alexa->getSetTime();
  alexa->refresh_lock();

  struct timeval current_set_time = alexa->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL
    supla_trivial_https *https = new supla_trivial_https();

    char host[] = "xatgh8yc1j.execute-api.eu-west-1.amazonaws.com";
    char resource[] = "/default/amazonRefreshTokenBridge";

    https->setHost(host);
    https->setResource(resource);
    {
      char *refresh_token = alexa->getRefreshToken();
      if (refresh_token) {
        int rtoken_len = strnlen(refresh_token, TOKEN_MAXSIZE);
        if (rtoken_len != 0) {
          cJSON *root = cJSON_CreateObject();
          cJSON_AddStringToObject(root, "refresh_token", refresh_token);
          char *str = cJSON_PrintUnformatted(root);
          cJSON_Delete(root);

          https->http_post(NULL, str);
          free(str);
        }

        free(refresh_token);
      }
    }

    if (https->getResultCode() == 200 && https->getBody()) {
      cJSON *root = cJSON_Parse(https->getBody());
      if (root) {
        cJSON *access_token = cJSON_GetObjectItem(root, "access_token");
        cJSON *expires_in = cJSON_GetObjectItem(root, "expires_in");
        cJSON *refresh_token = cJSON_GetObjectItem(root, "refresh_token");

        if (access_token && cJSON_IsString(access_token) && refresh_token &&
            cJSON_IsString(refresh_token)) {
          int ex_in = cJSON_IsNumber(expires_in) ? expires_in->valueint : 0;
          alexa->update(cJSON_GetStringValue(access_token),
                        cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }

    delete https;
#endif /*NOSSL*/
  }

  alexa->refresh_unlock();
}

int supla_alexa_client::aeg_post_request(char *data, int *httpResultCode) {
  int result = POST_RESULT_UNKNOWN_ERROR;

  if (httpResultCode) {
    *httpResultCode = 0;
  }

#ifdef NOSSL
  return POST_RESULT_NOSSL;
#else
  supla_trivial_https *https = new supla_trivial_https();

  {
    char host[30];
    host[0] = 0;
    char *region = alexa->getRegion();

    snprintf(host, 30, "api.%s%samazonalexa.com", region ? region : "",
             region ? "." : "");
    https->setHost(host);

    if (region) {
      free(region);
    }
  }

  char header[] = "Content-Type: application/json";
  char resource[] = "/v3/events";
  https->setResource(resource);
  https->setToken(alexa->getAccessToken(), false);

  if (!https->http_post(header, data)) {
    return POST_RESULT_REQUEST_ERROR;
  } else {
    if (httpResultCode) {
      *httpResultCode = https->getResultCode();
    }
    if (https->getResultCode() != 200) {
      if (https->getBody()) {
        cJSON *root = cJSON_Parse(https->getBody());
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

  delete https;

#endif /*NOSSL*/

  return result;
}

int supla_alexa_client::aeg_post(char *data) {
  if (!alexa->isAccessTokenExists()) {
    return POST_RESULT_TOKEN_DOES_NOT_EXISTS;
  }

  bool refresh_attempt = false;

  if (alexa->expiresIn() <= 30) {
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
    alexa->remove();
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

void *supla_alexa_client::getContactSensorProperties(bool hi) {
  char now[64];
  st_get_zulu_time(now);

  cJSON *property = cJSON_CreateObject();
  if (property) {
    cJSON_AddStringToObject(property, "namespace", "Alexa.ContactSensor");
    cJSON_AddStringToObject(property, "name", "detectionState");
    cJSON_AddStringToObject(property, "value",
                            hi ? "DETECTED" : "NOT_DETECTED");
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

void *supla_alexa_client::getChangeReportHeader(void) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    char msgId[37];
    msgId[0] = 0;
    st_uuid_v4(msgId);

    cJSON_AddStringToObject(header, "messageId", msgId);
    cJSON_AddStringToObject(header, "namespace", "Alexa");
    cJSON_AddStringToObject(header, "name", "ChangeReport");
    cJSON_AddStringToObject(header, "payloadVersion", "3");
  }

  return header;
}

void *supla_alexa_client::getEndpoint(int channelId) {
  cJSON *endpoint = cJSON_CreateObject();
  if (endpoint) {
    char *token = alexa->getAccessToken();
    if (token) {
      cJSON *scope = cJSON_CreateObject();
      if (scope) {
        cJSON_AddStringToObject(scope, "type", "BearerToken");
        cJSON_AddStringToObject(scope, "token", token);
        cJSON_AddItemToObject(endpoint, "scope", scope);
      }

      free(token);
    }

    char *escope = alexa->getEndpointScope();

    int endpointId_len =
        (escope ? strnlen(escope, ENDPOINTSCOPE_MAXSIZE) : 0) + 20;
    char *endpointId = (char *)malloc(endpointId_len + 1);

    if (endpointId) {
      snprintf(endpointId, endpointId_len, "%s-%i", escope ? escope : "",
               channelId);
      cJSON_AddStringToObject(endpoint, "endpointId", endpointId);
      free(endpointId);
    }

    if (escope) {
      free(escope);
    }
  }

  return endpoint;
}

void *supla_alexa_client::getChangeReport(int channelId, int cause_type,
                                          void *context_properties,
                                          void *change_properties) {
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

      cJSON *endpoint = static_cast<cJSON *>(getEndpoint(channelId));
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
bool supla_alexa_client::sendChangeReport(int channelId, bool hi, bool online,
                                          bool sensor) {
  char *data = NULL;

  if (!online) {
    hi = false;
  }

  void *context_props = NULL;

  if (online) {
    context_props =
        addProps(context_props, sensor ? getContactSensorProperties(hi)
                                       : getPowerControllerProperties(hi));

    context_props = addProps(context_props, getEndpointHealthProperties(true));
  }

  void *change_props = NULL;
  if (online) {
    change_props =
        addProps(change_props, sensor ? getContactSensorProperties(hi)
                                      : getPowerControllerProperties(hi));
  } else {
    change_props = addProps(change_props, getEndpointHealthProperties(online));
  }

  cJSON *root = static_cast<cJSON *>(
      getChangeReport(channelId, 0, context_props, change_props));

  if (root) {
    data = cJSON_Print(root);
    cJSON_Delete(root);
  }

  int result = 0;  // aeg_post(data);

  if (data) {
    supla_log(LOG_DEBUG, "%s", data);
    free(data);
    data = NULL;
  }

  return result == POST_RESULT_SUCCESS;
}
