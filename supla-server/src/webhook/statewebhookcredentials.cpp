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
  this->functions_ids.clear();
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

void supla_state_webhook_credentials::remove(void) {
  set(NULL, NULL, 0, NULL, NULL);
  database *db = new database();

  if (db->connect()) {
    db->state_webhook_remove_token(getUserID());
  }

  delete db;
}

void supla_state_webhook_credentials::set(const char *access_token,
                                          const char *refresh_token,
                                          int expires_in, const char *url,
                                          const char *functions_ids) {
  data_lock();
  url_free();
  this->functions_ids.clear();

  int url_len = url ? strnlen(url, WEBHOOK_URL_MAXSIZE) : 0;

  if (url_len > 0) {
    this->url = strndup(url, url_len);
  }

  if (functions_ids) {
    int len = strnlen(functions_ids, WEBHOOK_FUNCTIONS_IDS_MAXSIZE);
    int last_pos = 0;
    int function = 0;
    char divider = 1;
    for (int a = 0; a < len; a++) {
      if (functions_ids[a] >= '0' && functions_ids[a] <= '9') {
        function = function * 10 + functions_ids[a] - '0';
      } else if (functions_ids[a] == '-' && a == last_pos) {
        divider = -1;
      } else {
        if (function > 0) {
          this->functions_ids.push_back(function * divider);
        }

        last_pos = a + 1;
        function = 0;
        divider = 1;
      }
    }

    if (function > 0) {
      this->functions_ids.push_back(function * divider);
    }
  }

  supla_webhook_basic_credentials::set(access_token, refresh_token, expires_in);

  data_unlock();
}

void supla_state_webhook_credentials::update(const char *access_token,
                                             const char *refresh_token,
                                             int expires_in) {
  supla_webhook_basic_credentials::set(access_token, refresh_token, expires_in);

  database *db = new database();

  if (db->connect()) {
    db->state_webhook_update_token(getUserID(), access_token, refresh_token,
                                   expires_in);
  }

  delete db;
}

char *supla_state_webhook_credentials::getUrl(void) {
  char *result = NULL;

  data_lock();

  if (url != NULL) {
    result = strndup(url, WEBHOOK_URL_MAXSIZE);
  }

  data_unlock();

  return result;
}

std::list<int> supla_state_webhook_credentials::getFunctionsIds(void) {
  std::list<int> result;

  data_lock();
  result = functions_ids;
  data_unlock();

  return result;
}

void supla_state_webhook_credentials::on_credentials_changed() { load(); }

bool supla_state_webhook_credentials::isUrlProtocolAccepted(void) {
  bool result = false;
  data_lock();
  if (url) {
    int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
    if (len >= 8) {
      result = (url[0] == 'h' || url[0] == 'H') &&
               (url[1] == 't' || url[1] == 'T') &&
               (url[2] == 't' || url[2] == 'T') &&
               (url[3] == 'p' || url[3] == 'P') &&
               (url[4] == 's' || url[4] == 'S') && url[5] == ':' &&
               url[6] == '/' && url[7] == '/';
    }
  }
  data_unlock();
  return result;
}

bool supla_state_webhook_credentials::isUrlValid(void) {
  return isUrlProtocolAccepted() && strnlen(url, WEBHOOK_URL_MAXSIZE) > 11;
}

char *supla_state_webhook_credentials::getHost(void) {
  if (!isUrlValid()) {
    return NULL;
  }

  data_lock();
  char *result = NULL;

  int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
  for (int a = 8; a < len; a++) {
    if (url[a] == '/' || a == len - 1) {
      len = a == len - 1 ? a - 6 : a - 7;
      result = (char *)malloc(len);
      memcpy(result, &url[8], len - 1);
      result[len - 1] = 0;
      break;
    }
  }
  data_unlock();

  return result;
}

char *supla_state_webhook_credentials::getResource(void) {
  if (!isUrlValid()) {
    return NULL;
  }

  data_lock();
  char *result = NULL;
  int slash_pos = 0;

  int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
  for (int a = 8; a < len; a++) {
    if (slash_pos == 0 && url[a] == '/') {
      slash_pos = a;
    }
    if (a == len - 1) {
      if (slash_pos == a || slash_pos == 0) {
        result = strdup("/");
      } else {
        result = (char *)malloc(len - slash_pos + 1);
        memcpy(result, &url[slash_pos], len - slash_pos);
        result[len - slash_pos] = 0;
      }
      break;
    }
  }
  data_unlock();

  return result;
}
