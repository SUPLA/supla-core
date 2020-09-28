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

#include <amazon/alexacredentials.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "lck.h"
#include "log.h"
#include "user.h"

supla_amazon_alexa_credentials::supla_amazon_alexa_credentials(supla_user *user)
    : supla_webhook_basic_credentials(user) {
  this->region = NULL;
  set(NULL, NULL, 0, NULL);
}

supla_amazon_alexa_credentials::~supla_amazon_alexa_credentials() {
  region_free();
}

void supla_amazon_alexa_credentials::region_free(void) {
  if (this->region) {
    free(this->region);
    this->region = NULL;
  }
}

int supla_amazon_alexa_credentials::get_token_maxsize(void) {
  return ALEXA_TOKEN_MAXSIZE;
}

int supla_amazon_alexa_credentials::get_region_maxsize(void) {
  return ALEXA_REGION_MAXSIZE;
}

void supla_amazon_alexa_credentials::set(const char *access_token,
                                         const char *refresh_token,
                                         int expires_in, const char *region) {
  data_lock();
  region_free();

  int region_len = region ? strnlen(region, ALEXA_REGION_MAXSIZE) : 0;

  if (region_len > 0) {
    this->region = strndup(region, region_len);
  }

  supla_webhook_basic_credentials::set(access_token, refresh_token, expires_in);

  data_unlock();
}

void supla_amazon_alexa_credentials::load() {
  database *db = new database();

  if (!db->connect() || !db->amazon_alexa_load_credentials(this)) {
    set(NULL, NULL, 0, NULL);
  }

  delete db;
}

void supla_amazon_alexa_credentials::remove() {
  set(NULL, NULL, 0, NULL);
  database *db = new database();

  if (db->connect()) {
    db->amazon_alexa_remove_token(this);
  }

  delete db;
}

void supla_amazon_alexa_credentials::on_credentials_changed() { load(); }

void supla_amazon_alexa_credentials::update(const char *access_token,
                                            const char *refresh_token,
                                            int expires_in) {
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

char *supla_amazon_alexa_credentials::getRegion(void) {
  char *result = NULL;

  data_lock();

  if (region != NULL) {
    result = strndup(region, ALEXA_TOKEN_MAXSIZE);
  }

  data_unlock();

  return result;
}
