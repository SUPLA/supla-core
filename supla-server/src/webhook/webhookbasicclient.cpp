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
#include <webhook/webhookbasicclient.h>
#include <webhook/webhookbasiccredentials.h>
#include "http/trivialhttps.h"
#include "json/cJSON.h"
#include "lck.h"
#include "user/user.h"

supla_webhook_basic_client::supla_webhook_basic_client(
    supla_webhook_basic_credentials *credentials) {
  this->lck = lck_init();
  this->https = NULL;
  this->credentials = credentials;
}

void supla_webhook_basic_client::httpsInit(void) {
  httpsFree();
  lck_lock(lck);
  https = new supla_trivial_https();
  lck_unlock(lck);
}

void supla_webhook_basic_client::httpsFree(void) {
  lck_lock(lck);
  if (https) {
    delete https;
    https = NULL;
  }
  lck_unlock(lck);
}

void supla_webhook_basic_client::terminate(void) {
  lck_lock(lck);
  if (https) {
    https->terminate();
  }
  lck_unlock(lck);
}

supla_trivial_https *supla_webhook_basic_client::getHttps(void) {
  supla_trivial_https *result = NULL;
  lck_lock(lck);
  if (!https) {
    httpsInit();
  }
  result = https;
  lck_unlock(lck);
  return result;
}

supla_webhook_basic_credentials *supla_webhook_basic_client::getCredentials(
    void) {
  return credentials;
}

void supla_webhook_basic_client::refreshToken(char *host, char *resource) {
  if (!getCredentials()->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = getCredentials()->getSetTime();
  getCredentials()->refresh_lock();

  struct timeval current_set_time = getCredentials()->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL

    getHttps()->setHost(host);
    getHttps()->setResource(resource);
    {
      char *refresh_token = getCredentials()->getRefreshToken();
      if (refresh_token) {
        int rtoken_len =
            strnlen(refresh_token, getCredentials()->get_token_maxsize());
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
          getCredentials()->update(cJSON_GetStringValue(access_token),
                                   cJSON_GetStringValue(refresh_token), ex_in);
        }

        cJSON_Delete(root);
      }
    }

    httpsFree();

#endif /*NOSSL*/
  }

  getCredentials()->refresh_unlock();
}

supla_webhook_basic_client::~supla_webhook_basic_client() {
  httpsFree();
  lck_free(lck);
}
