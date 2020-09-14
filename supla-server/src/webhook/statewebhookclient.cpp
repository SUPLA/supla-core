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

bool supla_state_webhook_client::postRequest(const char *data) {
  if (!getStateWebhookCredentials()->isAccessTokenExists() ||
      !getStateWebhookCredentials()->isUrlValid()) {
    return false;
  }

  if (getStateWebhookCredentials()->expiresIn() <= 30) {
    refreshToken(getStateWebhookCredentials()->getHost(),
                 getStateWebhookCredentials()->getResource(), false);
  }

  return postRequest(data, NULL);
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

bool supla_state_webhook_client::sendOnOffReport(const char *function,
                                                 int channelId, bool on,
                                                 bool connected) {
  cJSON *root = getHeader(function, channelId);
  if (root) {
    cJSON *state = cJSON_CreateObject();
    if (state) {
      cJSON_AddBoolToObject(state, "on", on);
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
  return sendOnOffReport(function, channelId, on, connected);
}

bool supla_state_webhook_client::sendPowerSwitchReport(int channelId, bool on,
                                                       bool connected) {
  const char function[] = "POWERSWITCH";
  return sendOnOffReport(function, channelId, on, connected);
}
