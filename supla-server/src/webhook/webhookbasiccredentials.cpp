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
#include <webhook/webhookbasiccredentials.h>
#include "database.h"
#include "lck.h"
#include "log.h"
#include "user.h"

supla_webhook_basic_credentials::supla_webhook_basic_credentials(
    supla_user *user) {
  lck1 = lck_init();
  lck2 = lck_init();

  this->user = user;
  this->access_token = NULL;
  this->refresh_token = NULL;
  this->set_at.tv_sec = 0;
  this->set_at.tv_usec = 0;
  this->expires_at.tv_sec = 0;
  this->expires_at.tv_usec = 0;

  set(NULL, NULL, 0);
}

void supla_webhook_basic_credentials::access_token_free(void) {
  if (this->access_token) {
    free(this->access_token);
    this->access_token = NULL;
  }
}

void supla_webhook_basic_credentials::refresh_token_free(void) {
  if (this->refresh_token) {
    free(this->refresh_token);
    this->refresh_token = NULL;
  }
}

supla_webhook_basic_credentials::~supla_webhook_basic_credentials() {
  if (lck1) {
    lck_free(lck1);
    lck1 = NULL;
  }

  if (lck2) {
    lck_free(lck2);
    lck2 = NULL;
  }

  access_token_free();
  refresh_token_free();
}

int supla_webhook_basic_credentials::getUserID() { return user->getUserID(); }

supla_user *supla_webhook_basic_credentials::getUser() { return user; }

void supla_webhook_basic_credentials::data_lock(void) { lck_lock(lck1); }

void supla_webhook_basic_credentials::data_unlock(void) { lck_unlock(lck1); }

void supla_webhook_basic_credentials::refresh_lock(void) { lck_lock(lck2); }

void supla_webhook_basic_credentials::refresh_unlock(void) { lck_unlock(lck2); }

bool supla_webhook_basic_credentials::isAccessTokenExists(void) {
  bool result = false;

  data_lock();
  result = access_token != NULL;
  data_unlock();

  return result;
}

bool supla_webhook_basic_credentials::isRefreshTokenExists(void) {
  bool result = false;

  data_lock();
  result = refresh_token != NULL;
  data_unlock();

  return result;
}

char *supla_webhook_basic_credentials::getAccessToken(void) {
  char *result = NULL;

  data_lock();

  if (access_token != NULL) {
    result = strndup(access_token, get_token_maxsize());
  }

  data_unlock();

  return result;
}

char *supla_webhook_basic_credentials::getRefreshToken(void) {
  char *result = NULL;

  data_lock();

  if (refresh_token != NULL) {
    result = strndup(refresh_token, get_token_maxsize());
  }

  data_unlock();

  return result;
}

int supla_webhook_basic_credentials::expiresIn(void) {
  int result = 0;

  data_lock();

  struct timeval now;
  gettimeofday(&now, NULL);
  result = expires_at.tv_sec - now.tv_sec;

  data_unlock();

  return result;
}

struct timeval supla_webhook_basic_credentials::getSetTime(void) {
  struct timeval result;
  result.tv_sec = 0;
  result.tv_usec = 0;

  data_lock();
  result = set_at;
  data_unlock();

  return result;
}

void supla_webhook_basic_credentials::set(const char *access_token) {
  data_lock();
  access_token_free();

  int token_len = access_token ? strnlen(access_token, get_token_maxsize()) : 0;
  if (token_len > 0) {
    this->access_token = strndup(access_token, token_len);
  }

  gettimeofday(&set_at, NULL);

  data_unlock();
}

void supla_webhook_basic_credentials::set(const char *access_token,
                                          const char *refresh_token,
                                          int expires_in) {
  data_lock();
  refresh_token_free();

  supla_webhook_basic_credentials::set(access_token);

  int refresh_token_len =
      refresh_token ? strnlen(refresh_token, ALEXA_TOKEN_MAXSIZE) : 0;

  if (refresh_token_len > 0) {
    this->refresh_token = strndup(refresh_token, refresh_token_len);
  }

  if (expires_in == 0) {
    expires_in = 3600 * 24 * 365 * 10;
  }

  struct timeval set_at = getSetTime();

  this->expires_at.tv_sec = set_at.tv_sec + expires_in;
  this->expires_at.tv_usec = set_at.tv_usec;

  data_unlock();
}
