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

#include "webhook/statewebhookcredentials.h"

#include <stdlib.h>
#include <string.h>

#include "db/database.h"
#include "log.h"

using std::list;

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

list<int> supla_state_webhook_credentials::getFunctionsIds(void) {
  list<int> result;

  data_lock();
  result = functions_ids;
  data_unlock();

  return result;
}

void supla_state_webhook_credentials::on_credentials_changed() { load(); }

urlScheme supla_state_webhook_credentials::getScheme(void) {
  urlScheme result = schemeNotAccepted;

  data_lock();
  if (url) {
    int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
    if (len >= 7) {
      if ((url[0] == 'h' || url[0] == 'H') &&
          (url[1] == 't' || url[1] == 'T') &&
          (url[2] == 't' || url[2] == 'T') &&
          (url[3] == 'p' || url[3] == 'P')) {
        int offset = 4;

        if (url[4] == 's' || url[4] == 'S') {
          offset = 5;
        }

        if (offset + 2 < len && url[offset] == ':' && url[offset + 1] == '/' &&
            url[offset + 2] == '/') {
          result = offset == 4 ? schemeHttp : schemeHttps;
        }
      }
    }
  }
  data_unlock();
  return result;
}

char *supla_state_webhook_credentials::getHost(void) {
  urlScheme scheme = getScheme();

  if (scheme == schemeNotAccepted) {
    return NULL;
  }

  data_lock();
  char *result = NULL;

  int scheme_offset = scheme == schemeHttps ? 8 : 7;

  int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
  int dest_len = 0;
  for (int a = scheme_offset; a < len; a++) {
    char c = url[a];

    if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '.' || c == '/' || c == ':')) {
      break;
    }

    if (c == '/' || c == ':') {
      dest_len = a - scheme_offset;
    } else if (a == len - 1) {
      dest_len = a - scheme_offset + 1;
    }

    if (dest_len) {
      result = (char *)malloc(dest_len + 1);
      memcpy(result, &url[scheme_offset], dest_len);
      result[dest_len] = 0;
      break;
    }
  }
  data_unlock();

  return result;
}

char *supla_state_webhook_credentials::getResource(void) {
  urlScheme scheme = getScheme();

  if (scheme == schemeNotAccepted) {
    return NULL;
  }

  data_lock();
  char *result = NULL;
  int slash_pos = 0;

  int scheme_offset = scheme == schemeHttps ? 8 : 7;

  int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
  for (int a = scheme_offset; a < len; a++) {
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

int supla_state_webhook_credentials::getPort(void) {
  int result = 0;

  urlScheme scheme = getScheme();

  if (scheme == schemeNotAccepted) {
    return 0;
  }

  data_lock();
  bool colon = false;
  int scheme_offset = 0;

  if (scheme == schemeHttps) {
    scheme_offset = 8;
    result = 443;
  } else {
    scheme_offset = 7;
    result = 80;
  }

  int len = strnlen(url, WEBHOOK_URL_MAXSIZE);
  char c = 0;
  for (int a = scheme_offset; a < len; a++) {
    c = url[a];

    if (colon) {
      if (!((c >= '0' && c <= '9') || c == '/')) {
        result = 0;
        break;
      }

      if (c != '/') {
        result = result * 10 + c - '0';
      }
    }

    if (!colon && c == ':') {
      result = 0;
      colon = true;
    }

    if (a == len - 1 || c == '/') {
      break;
    }
  }
  data_unlock();

  return result;
}

bool supla_state_webhook_credentials::isUrlValid(void) {
  // This validation checks only a few cases.
  bool result = false;
  char *host = getHost();
  if (host) {
    result = strnlen(host, WEBHOOK_URL_MAXSIZE) > 0 && getPort() != 0;
    free(host);
  }

  return result;
}
