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
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "lck.h"
#include "log.h"
#include "user.h"

supla_amazon_alexa::supla_amazon_alexa(supla_user *user)
    : supla_voice_assistant(user) {
  this->refresh_token = NULL;
  this->region = NULL;
  this->expires_at.tv_sec = 0;
  this->expires_at.tv_usec = 0;

  set(NULL, NULL, 0, NULL);
}

supla_amazon_alexa::~supla_amazon_alexa() { strings_free(); };

void supla_amazon_alexa::strings_free(void) {
  if (this->refresh_token) {
    free(this->refresh_token);
    this->refresh_token = NULL;
  }

  if (this->region) {
    free(this->region);
    this->region = NULL;
  }
}

int supla_amazon_alexa::get_token_maxsize(void) { return ALEXA_TOKEN_MAXSIZE; }

void supla_amazon_alexa::set(const char *access_token,
                             const char *refresh_token, int expires_in,
                             const char *region) {
  data_lock();
  strings_free();

  supla_voice_assistant::set(access_token);

  int refresh_token_len =
      refresh_token ? strnlen(refresh_token, ALEXA_TOKEN_MAXSIZE) : 0;
  int region_len = region ? strnlen(region, ALEXA_REGION_MAXSIZE) : 0;

  if (refresh_token_len > 0) {
    this->refresh_token = strndup(refresh_token, refresh_token_len);
  }

  if (region_len > 0) {
    this->region = strndup(region, region_len);
  }

  if (expires_in == 0) {
    expires_in = 3600 * 24 * 365 * 10;
  }

  struct timeval set_at = getSetTime();

  this->expires_at.tv_sec = set_at.tv_sec + expires_in;
  this->expires_at.tv_usec = set_at.tv_usec;

  data_unlock();
}

void supla_amazon_alexa::load() {
  database *db = new database();

  if (!db->connect() || !db->amazon_alexa_load_token(this)) {
    set(NULL, NULL, 0, NULL);
  }

  delete db;
}

void supla_amazon_alexa::remove() {
  set(NULL, NULL, 0, NULL);
  database *db = new database();

  if (db->connect()) {
    db->amazon_alexa_remove_token(this);
  }

  delete db;
}

void supla_amazon_alexa::on_credentials_changed() { load(); }

void supla_amazon_alexa::update(const char *access_token,
                                const char *refresh_token, int expires_in) {
  char *region = getRegion();

  set(access_token, refresh_token, expires_in, region);

  if (region) {
    free(region);
  }

  database *db = new database();

  if (db->connect()) {
    db->amazon_alexa_update_token(this, access_token, refresh_token,
                                  expires_in);
  }

  delete db;
}

bool supla_amazon_alexa::isRefreshTokenExists(void) {
  bool result = false;

  data_lock();
  result = refresh_token != NULL;
  data_unlock();

  return result;
}

int supla_amazon_alexa::expiresIn(void) {
  int result = 0;

  data_lock();

  struct timeval now;
  gettimeofday(&now, NULL);
  result = expires_at.tv_sec - now.tv_sec;

  data_unlock();

  return result;
}

char *supla_amazon_alexa::getRefreshToken(void) {
  char *result = NULL;

  data_lock();

  if (refresh_token != NULL) {
    result = strndup(refresh_token, ALEXA_TOKEN_MAXSIZE);
  }

  data_unlock();

  return result;
}

char *supla_amazon_alexa::getRegion(void) {
  char *result = NULL;

  data_lock();

  if (region != NULL) {
    result = strndup(region, ALEXA_TOKEN_MAXSIZE);
  }

  data_unlock();

  return result;
}
