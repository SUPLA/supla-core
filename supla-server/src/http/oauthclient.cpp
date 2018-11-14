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

#include <http/oauthclient.h>
#include <json/cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "log.h"

#define TOKEN_MAXSIZE 2048

oauth_client::oauth_client() {
  this->token = NULL;
  this->oauth_host = NULL;
  this->refresh_token = NULL;
  this->token_endpoint = NULL;
  this->expires_at = 0;
  this->token_is_expired = false;
}

oauth_client::~oauth_client() {
  setToken(NULL, 0);
  setRefreshToken(NULL);
}

void oauth_client::setToken(const char *token, int expires_at) {
  this->expires_at = expires_at;
  this->token_is_expired = false;
  set_string_variable(&this->token, TOKEN_MAXSIZE, token);
}

void oauth_client::setRefreshToken(const char *refresh_token) {
  set_string_variable(&this->refresh_token, TOKEN_MAXSIZE, refresh_token);
}

void oauth_client::setOAuthHost(const char *oauth_host) {
  set_string_variable(&this->oauth_host, HOST_MAXSIZE, oauth_host);
}

void oauth_client::setOAuthTokenEndpoint(const char *token_endpoint) {
  set_string_variable(&this->token_endpoint, RESOURCE_MAXSIZE, token_endpoint);
}

bool oauth_client::isTokenExpired(void) {
  if (token_is_expired) {
    return true;
  }

  if (this->expires_at > 0) {
    struct timeval now;
    gettimeofday(&now, NULL);
  }

  return token_is_expired;
}

bool oauth_client::isExpiredByResult(void) {
  bool result = false;

  if (getResultCode() == 401) {
    cJSON *root = cJSON_Parse(getBody());
    if (root) {
      cJSON *err_desc = cJSON_GetObjectItem(root, "error_description");
      if (err_desc) {
        char str[] = "The access token provided has expired.";
        char *v = cJSON_GetStringValue(err_desc);

        if (v && strncmp(str, v, sizeof(str) - 1) == 0) {
          result = true;
        }
      }

      cJSON_Delete(root);
    }
  }

  return result;
}

bool oauth_client::request(const char *method, const char *header,
                           const char *data) {
  char *_header = (char *)header;

  if (isTokenExpired()) {
    return false;
  }

  if (this->token) {
    int header_len = header ? strnlen(header, HEADER_MAXSIZE) : 0;
    int new_header_len = 30 + header_len + strnlen(token, TOKEN_MAXSIZE);
    _header = (char *)malloc(new_header_len);

    snprintf(_header, new_header_len, "%s%sAuthorization: Bearer %s",
             header ? header : "", header ? "\r\n" : "", token);
  }

  bool result = supla_trivial_https::request(method, _header, data);

  if (_header != header) {
    free(_header);
  }

  if (result && getBody() && isExpiredByResult()) {
    token_is_expired = true;
  }

  return result;
}
