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

#include <alexa/alexaclient.h>
#include <alexa/alexatoken.h>
#include <http/trivialhttps.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include "json/cJSON.h"
#include "log.h"
#include "svrcfg.h"

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

supla_alexa_client::supla_alexa_client(supla_alexa_token *alexa_token) {
  this->alexa_token = alexa_token;
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
  if (!alexa_token->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = alexa_token->getSetTime();
  alexa_token->refresh_lock();

  struct timeval current_set_time = alexa_token->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL
    supla_trivial_https *https = new supla_trivial_https();

    char host[] = "xatgh8yc1j.execute-api.eu-west-1.amazonaws.com";
    char resource[] = "/default/amazonRefreshTokenBridge";

    https->setHost(host);
    https->setResource(resource);
    {
      char *refresh_token = alexa_token->getRefreshToken();
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
          alexa_token->update(cJSON_GetStringValue(access_token),
                              cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }

    delete https;
#endif /*NOSSL*/
  }

  alexa_token->refresh_unlock();
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
    char *region = alexa_token->getRegion();

    snprintf(host, 30, "api.%s%samazonalexa.com", region ? region : "",
             region ? "." : "");
    https->setHost(host);

    if (region) {
      free(region);
    }
  }

  char resource[] = "/v3/events";
  https->setResource(resource);
  https->setToken(alexa_token->getToken(), false);

  if (!https->http_post(NULL, data)) {
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
  if (!alexa_token->isTokenExists()) {
    return POST_RESULT_TOKEN_DOES_NOT_EXISTS;
  }

  bool refresh_attempt = false;

  if (alexa_token->expiresIn() <= 30) {
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
    alexa_token->remove();
    return result;
  }

  if (result != POST_RESULT_SUCCESS) {
    supla_log(LOG_ERR, "Alexa event gateway client error (%i/%i): %s",
              httpResultCode, result, getErrorString(result));
  }

  return result;
}

bool supla_alexa_client::sendChangeReport(int channelId, bool hi, bool online,
                                          bool sensor) {
  char *data = NULL;

  if (!online) {
    hi = false;
  }

  cJSON *root = cJSON_CreateObject();
  if (root) {
    cJSON *context = cJSON_CreateObject();
    if (context) {
      cJSON *properties = cJSON_CreateObject();
      if (properties) {
        cJSON_AddStringToObject(
            properties, "namespace",
            sensor ? "Alexa.ContactSensor" : "Alexa.PowerController");

        cJSON_AddStringToObject(properties, "name",
                                sensor ? "detectionState" : "powerState");

        cJSON_AddStringToObject(
            properties, "value",
            sensor ? (hi ? "DETECTED" : "NOT_DETECTED") : (hi ? "ON" : "OFF"));

        cJSON_AddStringToObject(properties, "timeOfSample", "");

        cJSON_AddNumberToObject(properties, "uncertaintyInMilliseconds", 0);

        cJSON_AddItemToObject(context, "properties", properties);
      }

      cJSON_AddItemToObject(root, "context", context);
    }

    cJSON *event = cJSON_CreateObject();
    if (event) {
      cJSON *header = cJSON_CreateObject();
      if (header) {
        cJSON_AddItemToObject(event, "header", header);
      }

      cJSON *endpoint = cJSON_CreateObject();
      if (endpoint) {
        cJSON_AddItemToObject(event, "endpoint", endpoint);
      }

      cJSON_AddItemToObject(root, "event", event);
    }

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
