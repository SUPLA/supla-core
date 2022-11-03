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

#include "conn/authkey_cache.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safearray.h"
#include "svrcfg.h"

using std::function;

typedef struct {
  char guid[SUPLA_GUID_SIZE];
  char *email;
  char authkey[SUPLA_AUTHKEY_SIZE];
  int user_id;
  bool result;
} authkey_cache_item_t;

// static
supla_authkey_cache supla_authkey_cache::global_instance;

supla_authkey_cache::supla_authkey_cache(void) {
  items = safe_array_init();
  cache_size_limit = -1;
}

supla_authkey_cache::~supla_authkey_cache(void) {
  for (int a = 0; a < safe_array_count(items); a++) {
    authkey_cache_item_t *i =
        static_cast<authkey_cache_item_t *>(safe_array_get(items, a));
    if (i) {
      if (i->email) {
        free(i->email);
      }
      free(i);
    }
  }

  safe_array_free(items);
}

// static
supla_authkey_cache &supla_authkey_cache::get_global_instance(void) {
  return supla_authkey_cache::global_instance;
}

int supla_authkey_cache::get_cache_size_limit(void) {
  int result = 0;
  safe_array_lock(items);
  if (cache_size_limit == -1) {
    cache_size_limit = scfg_int(CFG_LIMIT_AUTHKEY_AUTH_CACHE_SIZE);
  }
  result = cache_size_limit;
  safe_array_unlock(items);

  return result >= 0 ? result : 0;
}

int supla_authkey_cache::get_size(void) { return safe_array_count(items); }

bool supla_authkey_cache::authkey_auth(const char guid[SUPLA_GUID_SIZE],
                                       const char email[SUPLA_EMAIL_MAXSIZE],
                                       const char authkey[SUPLA_AUTHKEY_SIZE],
                                       int *user_id,
                                       function<bool(void)> db_authkey_auth) {
  // The cache is designed to reduce the number of queries to the database, in
  // particular the number of calls to the function st_bcrypt_check, which
  // consumes CPU resources

  if (guid == nullptr || authkey == nullptr || email == nullptr ||
      user_id == nullptr) {
    return false;
  }

  int cache_size_limit = get_cache_size_limit();

  if (cache_size_limit > 0) {
    safe_array_lock(items);

    for (int a = 0; a < safe_array_count(items); a++) {
      authkey_cache_item_t *i =
          static_cast<authkey_cache_item_t *>(safe_array_get(items, a));
      if (i && strncmp(i->email, email, SUPLA_EMAIL_MAXSIZE) == 0 &&
          memcmp(i->guid, guid, SUPLA_GUID_SIZE) == 0 &&
          memcmp(i->authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0) {
        bool result = i->result;
        *user_id = i->user_id;
        safe_array_move_to_begin(items, a);
        safe_array_unlock(items);
        return result;
      }
    }

    safe_array_unlock(items);

    authkey_cache_item_t *i = NULL;
    bool result = db_authkey_auth();

    safe_array_lock(items);

    if (safe_array_count(items) >= cache_size_limit) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      int count = safe_array_count(items);
      int idx = count / 2;
      if (idx > 0) {
        idx = count - 1 - (tv.tv_usec % idx);
      }

      i = static_cast<authkey_cache_item_t *>(safe_array_get(items, idx));

    } else {
      i = (authkey_cache_item_t *)malloc(sizeof(authkey_cache_item_t));
      memset(i, 0, sizeof(authkey_cache_item_t));
      safe_array_add(items, i);
    }

    if (i) {
      memcpy(i->guid, guid, SUPLA_GUID_SIZE);
      memcpy(i->authkey, authkey, SUPLA_AUTHKEY_SIZE);

      if (i->email == NULL) {
        i->email = strndup(email, SUPLA_EMAIL_MAXSIZE);
      } else {
        int len1 = strnlen(email, SUPLA_EMAIL_MAXSIZE);
        int len2 = strnlen(i->email, SUPLA_EMAIL_MAXSIZE);
        if (len1 > len2) {
          i->email = (char *)realloc(i->email, len1 + 1);
        }
        snprintf(i->email, len1 + 1, "%s", email);
      }

      i->result = result;
      i->user_id = *user_id;
    }

    safe_array_unlock(items);
    return result;

  } else {
    return db_authkey_auth();
  }
}
