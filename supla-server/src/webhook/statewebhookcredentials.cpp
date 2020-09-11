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

#include "database.h"
#include "webhook/statewebhookcredentials.h"

supla_state_webhook_credentials::supla_state_webhook_credentials(
    supla_user *user)
    : supla_webhook_basic_credentials(user) {
  this->url = NULL;
  set(NULL, NULL, 0, NULL, NULL);
}

supla_state_webhook_credentials::~supla_state_webhook_credentials(void) {
  data_lock();
  url_free();
}

void supla_state_webhook_credentials::url_free() {
  if (url) {
    free(url);
    url = NULL;
  }
}

int supla_state_webhook_credentials::get_token_maxsize(void) {
  return WEBHOOK_TOKEN_MAXSIZE;
}

void supla_state_webhook_credentials::load() {
  database *db = new database();

  if (!db->connect() || !db->state_webhook_load_credentials(this)) {
    set(NULL, NULL, 0, NULL, NULL);
  }

  delete db;
}

void supla_state_webhook_credentials::set(const char *access_token,
                                          const char *refresh_token,
                                          int expires_in, const char *url,
                                          const char *functions_ids) {
  data_lock();
  url_free();

  int url_len = url ? strnlen(url, WEBHOOK_URL_MAXSIZE) : 0;

  if (url_len > 0) {
    this->url = strndup(url, url_len);
  }

  supla_webhook_basic_credentials::set(access_token, refresh_token, expires_in);

  data_unlock();
}

void supla_state_webhook_credentials::on_credentials_changed() { load(); }
