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

#include <stdlib.h>
#include <string.h>
#include <webhook/statewebhookclient.h>
#include "user/user.h"

supla_state_webhook_client::supla_state_webhook_client(
    supla_webhook_basic_credentials *credentials)
    : supla_webhook_basic_client(credentials) {}

supla_state_webhook_credentials *
supla_state_webhook_client::getStateWebhookCredentials(void) {
  return static_cast<supla_state_webhook_credentials *>(getCredentials());
}

bool supla_state_webhook_client::postRequest(const char *data,
                                             int *httpResultCode) {
  if (httpResultCode) {
    *httpResultCode = 0;
  }

#ifdef NOSSL
  return false;
#else

  char header[] = "Content-Type: application/json";
  bool result = false;

  getHttpConnection()->setHost(getStateWebhookCredentials()->getHost(), false);
  getHttpConnection()->setResource(getStateWebhookCredentials()->getResource(),
                                   false);
  getHttpConnection()->setToken(getStateWebhookCredentials()->getAccessToken(),
                                false);

  if (getHttpConnection()->http_post(header, data)) {
    if (httpResultCode) {
      *httpResultCode = getHttpConnection()->getResultCode();
    }

    result = getHttpConnection()->getResultCode() == 200 ||
             getHttpConnection()->getResultCode() == 202;
  }

  httpConnectionFree();

#endif /*NOSSL*/

  return result;
}

void supla_state_webhook_client::refreshToken(void) {
  if (!getCredentials()->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = getCredentials()->getSetTime();
  getCredentials()->refresh_lock();

  struct timeval current_set_time = getCredentials()->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL

    getHttpConnection()->setHost(getStateWebhookCredentials()->getHost(),
                                 false);
    getHttpConnection()->setResource(
        getStateWebhookCredentials()->getResource(), false);
    {
      char *refresh_token = getCredentials()->getRefreshToken();
      if (refresh_token) {
        int rtoken_len =
            strnlen(refresh_token, getCredentials()->get_token_maxsize());
        if (rtoken_len != 0) {
          cJSON *root = cJSON_CreateObject();
          cJSON_AddStringToObject(root, "refreshToken", refresh_token);
          char *shortUniqueId = getCredentials()->getUser()->getShortUniqueID();
          cJSON_AddStringToObject(root, "userShortUniqueId", shortUniqueId);
          free(shortUniqueId);
          char *str = cJSON_PrintUnformatted(root);
          cJSON_Delete(root);

          char header[] = "Content-Type: application/json";
          getHttpConnection()->http_put(header, str);
          free(str);
        }

        free(refresh_token);
      }
    }

    if (getHttpConnection()->getResultCode() == 200 &&
        getHttpConnection()->getBody()) {
      cJSON *root = cJSON_Parse(getHttpConnection()->getBody());
      if (root) {
        cJSON *access_token = cJSON_GetObjectItem(root, "accessToken");
        cJSON *expires_in = cJSON_GetObjectItem(root, "expiresIn");
        cJSON *refresh_token = cJSON_GetObjectItem(root, "refreshToken");

        if (access_token && cJSON_IsString(access_token) && refresh_token &&
            cJSON_IsString(refresh_token)) {
          int ex_in = cJSON_IsNumber(expires_in) ? expires_in->valueint : 0;
          getCredentials()->update(cJSON_GetStringValue(access_token),
                                   cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }

    httpConnectionFree();

#endif /*NOSSL*/
  }

  getCredentials()->refresh_unlock();
}

bool supla_state_webhook_client::postRequest(const char *data) {
  if (!getStateWebhookCredentials()->isAccessTokenExists() ||
      !getStateWebhookCredentials()->isUrlValid()) {
    return false;
  }

  bool refresh_attempt = false;

  if (getStateWebhookCredentials()->expiresIn() <= 30) {
    refresh_attempt = true;
    refreshToken();
  }

  int httpResultCode = 0;
  if (postRequest(data, &httpResultCode)) {
    return true;
  }

  if (refresh_attempt && httpResultCode == 403) {
    getStateWebhookCredentials()->remove();
  }

  return false;
}

bool supla_state_webhook_client::sendReport(cJSON *json) {
  if (json == NULL) {
    return false;
  }

  bool result = false;
#ifdef ONLY_LOG_REQUESTS
  char *data = cJSON_Print(json);
#else
  char *data = cJSON_PrintUnformatted(json);
#endif /*ONLY_LOG_REQUESTS*/
  cJSON_Delete(json);
  json = NULL;

  if (data != NULL) {
#ifdef ONLY_LOG_REQUESTS
    result = true;
#else
    result = postRequest(data);
#endif /*ONLY_LOG_REQUESTS*/
    free(data);
    data = NULL;
  }

  return result;
}

cJSON *supla_state_webhook_client::getHeader(const char *function,
                                             int channelId) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    char *shortUniqueId = getCredentials()->getUser()->getShortUniqueID();
    cJSON_AddStringToObject(header, "userShortUniqueId", shortUniqueId);
    free(shortUniqueId);

    char number[12];
    snprintf(number, sizeof(number), "%i", channelId);
    cJSON_AddStringToObject(header, "channelId", number);

    cJSON_AddStringToObject(header, "channelFunction", function);

#ifdef __TEST
    cJSON_AddStringToObject(header, "timestamp", "1600097258");
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    snprintf(number, sizeof(number), "%lu", now.tv_sec);
    cJSON_AddStringToObject(header, "timestamp", number);
#endif /*__TEST*/
  }

  return header;
}

bool supla_state_webhook_client::sendBinaryReport(const char *function,
                                                  int channelId,
                                                  const char *name, bool _true,
                                                  bool connected) {
  cJSON *root = getHeader(function, channelId);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      cJSON_AddBoolToObject(state, name, _true);
      cJSON_AddBoolToObject(state, "connected", connected);
      cJSON_AddItemToObject(root, "state", state);
      return sendReport(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::sendOnReport(const char *function,
                                              int channelId, bool on,
                                              bool connected) {
  return sendBinaryReport(function, channelId, "on", on, connected);
}

bool supla_state_webhook_client::sendHiReport(const char *function,
                                              int channelId, bool hi,
                                              bool connected) {
  return sendBinaryReport(function, channelId, "hi", hi, connected);
}

bool supla_state_webhook_client::sendTemperatureAndHumidityReport(
    const char *function, int channelId, double *temperature, double *humidity,
    bool connected) {
  cJSON *root = getHeader(function, channelId);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      if (temperature) {
        cJSON_AddNumberToObject(state, "temperature", *temperature);
      }

      if (humidity) {
        cJSON_AddNumberToObject(state, "humidity", *humidity);
      }

      cJSON_AddBoolToObject(state, "connected", connected);
      cJSON_AddItemToObject(root, "state", state);
      return sendReport(root);
    } else {
      cJSON_Delete(root);
    }
  }

  return false;
}

bool supla_state_webhook_client::sendLightSwitchReport(int channelId, bool on,
                                                       bool connected) {
  const char function[] = "LIGHTSWITCH";
  return sendOnReport(function, channelId, on, connected);
}

bool supla_state_webhook_client::sendPowerSwitchReport(int channelId, bool on,
                                                       bool connected) {
  const char function[] = "POWERSWITCH";
  return sendOnReport(function, channelId, on, connected);
}

bool supla_state_webhook_client::sendTemperatureReport(int channelId,
                                                       double temperature,
                                                       bool connected) {
  const char function[] = "THERMOMETER";
  return sendTemperatureAndHumidityReport(function, channelId, &temperature,
                                          NULL, connected);
}

bool supla_state_webhook_client::sendHumidityReport(int channelId,
                                                    double humidity,
                                                    bool connected) {
  const char function[] = "HUMIDITY";
  return sendTemperatureAndHumidityReport(function, channelId, NULL, &humidity,
                                          connected);
}

bool supla_state_webhook_client::sendTemperatureAndHumidityReport(
    int channelId, double temperature, double humidity, bool connected) {
  const char function[] = "HUMIDITYANDTEMPERATURE";
  return sendTemperatureAndHumidityReport(function, channelId, &temperature,
                                          &humidity, connected);
}

bool supla_state_webhook_client::sendGatewayOpeningSensorReport(
    int channelId, bool hi, bool connected) {
  const char function[] = "OPENINGSENSOR_GATEWAY";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendGateOpeningSensorReport(int channelId,
                                                             bool hi,
                                                             bool connected) {
  const char function[] = "OPENINGSENSOR_GATE";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendGarageDoorOpeningSensorReport(
    int channelId, bool hi, bool connected) {
  const char function[] = "OPENINGSENSOR_GARAGEDOOR";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendNoLiquidSensorReport(int channelId,
                                                          bool hi,
                                                          bool connected) {
  const char function[] = "NOLIQUIDSENSOR";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendDoorOpeningSensorReport(int channelId,
                                                             bool hi,
                                                             bool connected) {
  const char function[] = "OPENINGSENSOR_DOOR";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendRollerShutterOpeningSensorReport(
    int channelId, bool hi, bool connected) {
  const char function[] = "OPENINGSENSOR_ROLLERSHUTTER";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendWindowOpeningSensorReport(int channelId,
                                                               bool hi,
                                                               bool connected) {
  const char function[] = "OPENINGSENSOR_WINDOW";
  return sendHiReport(function, channelId, hi, connected);
}

bool supla_state_webhook_client::sendMailSensorReport(int channelId, bool hi,
                                                      bool connected) {
  const char function[] = "MAILSENSOR";
  return sendHiReport(function, channelId, hi, connected);
}
