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
  this->httpConnection = NULL;
  this->httpConnectionFactory = NULL;
  this->credentials = credentials;
}

void supla_webhook_basic_client::httpConnectionInit(void) {
  httpConnectionFree();
  lck_lock(lck);
  if (httpConnectionFactory) {
    httpConnection = httpConnectionFactory->createConnection();
  } else {
    httpConnection = new supla_trivial_https();
  }

  lck_unlock(lck);
}

void supla_webhook_basic_client::httpConnectionFree(void) {
  lck_lock(lck);
  if (httpConnection) {
    delete httpConnection;
    httpConnection = NULL;
  }
  lck_unlock(lck);
}

void supla_webhook_basic_client::terminate(void) {
  lck_lock(lck);
  if (httpConnection) {
    httpConnection->terminate();
  }
  lck_unlock(lck);
}

supla_trivial_http *supla_webhook_basic_client::getHttpConnection(void) {
  supla_trivial_http *result = NULL;
  lck_lock(lck);
  if (!httpConnection) {
    httpConnectionInit();
  }
  result = httpConnection;
  lck_unlock(lck);
  return result;
}

void supla_webhook_basic_client::setHttpConnectionFactory(
    supla_trivial_http_factory *httpConnectionFactory) {
  lck_lock(lck);
  this->httpConnectionFactory = httpConnectionFactory;
  lck_unlock(lck);
}

supla_trivial_http_factory *
supla_webhook_basic_client::getHttpConnectionFactory(void) {
  supla_trivial_http_factory *result = NULL;
  lck_lock(lck);
  result = httpConnectionFactory;
  lck_unlock(lck);

  return result;
}

supla_webhook_basic_credentials *supla_webhook_basic_client::getCredentials(
    void) {
  return credentials;
}

void supla_webhook_basic_client::refreshToken(char *host, char *resource,
                                              bool copy, const char *body,
                                              bool put) {
  if (!getCredentials()->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = getCredentials()->getSetTime();
  getCredentials()->refresh_lock();

  struct timeval current_set_time = getCredentials()->getSetTime();
  if (last_set_time.tv_sec == current_set_time.tv_sec &&
      last_set_time.tv_usec == current_set_time.tv_usec) {
#ifndef NOSSL

    getHttpConnection()->setHost(host, copy);
    getHttpConnection()->setResource(resource, copy);
    if (body) {
      if (put) {
        getHttpConnection()->http_put(NULL, body);
      } else {
        getHttpConnection()->http_post(NULL, body);
      }

    } else {
      char *refresh_token = getCredentials()->getRefreshToken();
      if (refresh_token) {
        int rtoken_len =
            strnlen(refresh_token, getCredentials()->get_token_maxsize());
        if (rtoken_len != 0) {
          cJSON *root = cJSON_CreateObject();
          cJSON_AddStringToObject(root, "refresh_token", refresh_token);
          char *str = cJSON_PrintUnformatted(root);
          cJSON_Delete(root);

          if (put) {
            getHttpConnection()->http_put(NULL, str);
          } else {
            getHttpConnection()->http_post(NULL, str);
          }

          free(str);
        }

        free(refresh_token);
      }
    }

    if (getHttpConnection()->getResultCode() == 200 &&
        getHttpConnection()->getBody()) {
      cJSON *root = cJSON_Parse(getHttpConnection()->getBody());
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

    httpConnectionFree();

#endif /*NOSSL*/
  }

  getCredentials()->refresh_unlock();
}

supla_webhook_basic_client::~supla_webhook_basic_client() {
  httpConnectionFree();
  lck_free(lck);
}
