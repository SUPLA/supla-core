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

#include "google/googlehomeclient.h"
#include <stdlib.h>
#include "http/trivialhttps.h"
#include "json/cJSON.h"
#include "string.h"
#include "tools.h"
#include "user/user.h"

supla_google_home_client::supla_google_home_client(
    supla_google_home *google_home)
    : supla_voice_assistant_client(google_home) {
  jsonStates = cJSON_CreateObject();
}

supla_google_home_client::~supla_google_home_client(void) {
  cJSON_Delete((cJSON *)jsonStates);
}

void supla_google_home_client::clear(void) {
  cJSON_Delete((cJSON *)jsonStates);
  jsonStates = cJSON_CreateObject();
}

bool supla_google_home_client::post(char *data, const char host[],
                                    const char resource[]) {
  int result = false;

#ifdef NOSSL
  return false;
#else
  {
    getHttps()->setHost((char *)host, true);
    getHttps()->setResource((char *)resource, true);
  }

  getHttps()->setToken(getVoiceAssistant()->getAccessToken(), false);
  result =
      getHttps()->http_post(NULL, data) && getHttps()->getResultCode() == 200;

  httpsFree();

#endif /*NOSSL*/

  return result;
}

bool supla_google_home_client::channelExists(const char *endpointId) {
  return endpointId != NULL &&
         cJSON_GetObjectItem((cJSON *)jsonStates, endpointId) != NULL;
}

void supla_google_home_client::requestSync(void) {}

void *supla_google_home_client::getStateSkeleton(int channelId,
                                                 short subChannel,
                                                 bool online) {
  cJSON *state = NULL;
  char *endpointId = getEndpointId(channelId, 0);
  if (endpointId) {
    if (!channelExists(endpointId)) {
      state = cJSON_CreateObject();
      if (state) {
        cJSON_AddBoolToObject(state, "online", online);
        cJSON_AddItemToObject((cJSON *)jsonStates, endpointId, state);
      }
    }
    free(endpointId);
  }

  return state;
}

bool supla_google_home_client::addOnOffState(int channelId, bool on,
                                             bool online) {
  cJSON *state = (cJSON *)getStateSkeleton(channelId, 0, online);
  if (state) {
    cJSON_AddBoolToObject(state, "on", online && on);
    return true;
  }

  return false;
}

bool supla_google_home_client::addBrightnessState(int channelId,
                                                  short brightness, bool online,
                                                  short subChannel) {
  cJSON *state = (cJSON *)getStateSkeleton(channelId, subChannel, online);
  if (state) {
    cJSON_AddBoolToObject(state, "on", online && brightness > 0);
    cJSON_AddNumberToObject(state, "brightness", brightness);
    return true;
  }

  return false;
}

bool supla_google_home_client::addColorState(int channelId, int color,
                                             short colorBrightness, bool online,
                                             short subChannel) {
  cJSON *state = (cJSON *)getStateSkeleton(channelId, subChannel, online);
  if (state) {
    cJSON *jsonColor = cJSON_CreateObject();
    if (jsonColor) {
      cJSON_AddNumberToObject(jsonColor, "spectrumRgb", color);
      cJSON_AddItemToObject(state, "color", jsonColor);

      cJSON_AddBoolToObject(state, "on", online && colorBrightness > 0);
      cJSON_AddNumberToObject(state, "brightness", colorBrightness);
    }

    return true;
  }

  return false;
}

void *supla_google_home_client::getHeader(const char requestId[]) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    const char name[] = "requestId";
    if (requestId && strnlen(requestId, 1024) > 0) {
      cJSON_AddStringToObject(header, name, requestId);
    } else {
      char reqId[37];
      reqId[0] = 0;
      st_uuid_v4(reqId);
      cJSON_AddStringToObject(header, name, reqId);
    }

    char *longUniqueId = getVoiceAssistant()->getUser()->getLongUniqueID();
    if (longUniqueId) {
      cJSON_AddStringToObject(header, "agentUserId", longUniqueId);
      free(longUniqueId);
    }

    return header;
  }

  return NULL;
}

bool supla_google_home_client::sendReportState(const char requestId[]) {
  bool result = false;
  cJSON *report = (cJSON *)getHeader(requestId);

  if (report) {
    cJSON *payload = cJSON_CreateObject();
    if (payload) {
      cJSON *devices = cJSON_CreateObject();
      if (devices) {
        cJSON_AddItemToObject(devices, "states", (cJSON *)jsonStates);
        cJSON_AddItemToObject(payload, "devices", devices);

        char *data = cJSON_Print(report);
        if (data) {
          char resource[] = "";
          char host[] = "";
          result = post(data, host, resource);
          free(data);
        }
      }

      cJSON_AddItemToObject(report, "payload", payload);
    }
  }

  return result;
}
