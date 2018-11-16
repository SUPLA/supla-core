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
#include <string.h>
#include <map>
#include <string>
#include "json/cJSON.h"
#include "log.h"
#include "svrcfg.h"

#define POST_RESULT_SUCCESS 1
#define POST_RESULT_UNKNOWN_ERROR 0

#define POST_RESULT_INVALID_REQUEST_EXCEPTION -1
#define POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION -2
#define POST_RESULT_SKILL_DISABLED_EXCEPTION -3
#define POST_RESULT_INSUFFICIENT_PERMISSION_EXCEPTION -4
#define POST_RESULT_SKILL_NOT_FOUND_EXCEPTION 1005
#define POST_RESULT_REQUEST_ENTITY_TOO_LARGE_EXCEPTION -5
#define POST_RESULT_THROTTLING_EXCEPTION -6
#define POST_RESULT_INTERNAL_SERVICE_EXCEPTION -7
#define POST_RESULT_SERVICE_UNAVAILABLE_EXCEPTION -8

#define POST_RESULT_NOSSL -100
#define POST_RESULT_TOKEN_DOES_NOT_EXISTS -200

typedef struct {
  char *str;
  int code;
} _alexa_code_t;

// https://developer.amazon.com/docs/smarthome/send-events-to-the-alexa-event-gateway.html
static const _alexa_code_t alexa_codes[]{
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

int supla_alexa_client::parseErrorCode(const char *code) {
  int n = 0;
  while (alexa_codes[n].str) {
    if (strncmp(alexa_codes[n].str, code, 50) == 0) {
      return alexa_codes[n].code;
    }
    n++;
  };

  return POST_RESULT_UNKNOWN_ERROR;
}

supla_alexa_client::~supla_alexa_client() {}

void supla_alexa_client::refresh_roken() {
  if (!alexa_token->isRefreshTokenExists()) {
    return;
  }

  struct timeval last_set_time = alexa_token->getSetTime();
  alexa_token->refresh_lock();

  struct timeval current_set_time = alexa_token->getSetTime();
  if (last_set_time.tv_sec != current_set_time.tv_sec ||
      last_set_time.tv_usec != current_set_time.tv_usec) {
    char *token = NULL;
    char *refresh_roken = NULL;
    int expires_at = 0;

#ifndef NOSSL
    supla_trivial_https *https = new supla_trivial_https();

    char host[] = "api.amazon.com";
    char resource[] = "/auth/o2/token";

    https->setHost(host);
    https->setResource(resource);
    https->setToken(alexa_token->getToken(), false);

    delete https;
#endif /*NOSSL*/
  }

  alexa_token->refresh_unlock();
}

int supla_alexa_client::aeg_post_request(char *data) {
  int result = POST_RESULT_UNKNOWN_ERROR;

#ifdef NOSSL
  return POST_RESULT_NOSSL;
#else
  supla_trivial_https *https = new supla_trivial_https();

  https->setHost(scfg_string(CFG_ALEXA_EVENT_GATEWAY_HOST));

  char resource[] = "/v3/events";
  https->setResource(resource);
  https->setToken(alexa_token->getToken(), false);

  https->http_post();

  supla_log(LOG_DEBUG, "%s", https->getBody());
  if (https->getResultCode() != 200 && https->getBody()) {
    cJSON *root = cJSON_Parse(https->getBody());
    if (root) {
      cJSON *payload = cJSON_GetObjectItem(root, "payload");
      if (payload) {
        cJSON *code = cJSON_GetObjectItem(payload, "code");
        if (code) {
          result = parseErrorCode(cJSON_GetStringValue(code));
        }
      }
      cJSON_Delete(root);
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

  int result = aeg_post_request(data);
  if (result == POST_RESULT_INVALID_ACCESS_TOKEN_EXCEPTION) {
    if (!refresh_attempt) {
      refresh_roken();
      result = aeg_post_request(data);
    }
  } else if (result == POST_RESULT_SKILL_DISABLED_EXCEPTION ||
             result == POST_RESULT_SKILL_NOT_FOUND_EXCEPTION) {
    alexa_token->remove();
  }

  return result;
}

void supla_alexa_client::test(void) { aeg_post(NULL); }
