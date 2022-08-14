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

#include "cdbase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lck.h"
#include "safearray.h"
#include "svrcfg.h"
#include "user.h"

typedef struct {
  char guid[SUPLA_GUID_SIZE];
  char *email;
  char authkey[SUPLA_AUTHKEY_SIZE];
  int user_id;
  bool result;
} authkey_cache_item_t;

void *cdbase::authkey_auth_cache_arr = NULL;
int cdbase::authkey_auth_cache_size = 0;

// static
void cdbase::init(void) {
  cdbase::authkey_auth_cache_size = scfg_int(CFG_LIMIT_AUTHKEY_AUTH_CACHE_SIZE);
  cdbase::authkey_auth_cache_arr = safe_array_init();
}

// static
void cdbase::cdbase_free(void) {
  for (int a = 0; a < safe_array_count(cdbase::authkey_auth_cache_arr); a++) {
    authkey_cache_item_t *i = static_cast<authkey_cache_item_t *>(
        safe_array_get(cdbase::authkey_auth_cache_arr, a));
    if (i) {
      if (i->email) {
        free(i->email);
      }
      free(i);
    }
  }

  safe_array_free(cdbase::authkey_auth_cache_arr);
}

cdbase::cdbase(supla_connection *conn) {
  this->user = NULL;
  this->conn = conn;
  this->lck = lck_init();
  this->ID = 0;
  this->ptr_counter = 0;
  memset(this->guid, 0, SUPLA_GUID_SIZE);
  memset(this->authkey, 0, SUPLA_AUTHKEY_SIZE);

  update_last_activity();  // last line / after lck_init
}

cdbase::~cdbase() { lck_free(this->lck); }

void cdbase::terminate(void) {
  lck_lock(lck);
  if (conn) conn->terminate();
  lck_unlock(lck);
}

bool cdbase::set_guid(char guid[SUPLA_GUID_SIZE]) {
  char _guid[SUPLA_GUID_SIZE] = {};

  if (memcmp(_guid, guid, SUPLA_GUID_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->guid, guid, SUPLA_GUID_SIZE);
  lck_unlock(lck);

  return true;
}

void cdbase::get_guid(char guid[SUPLA_GUID_SIZE]) {
  lck_lock(lck);
  memcpy(guid, this->guid, SUPLA_GUID_SIZE);
  lck_unlock(lck);
}

bool cdbase::cmp_guid(const char guid[SUPLA_GUID_SIZE]) {
  bool result = false;

  lck_lock(lck);
  result = memcmp(this->guid, guid, SUPLA_GUID_SIZE) == 0;
  lck_unlock(lck);

  return result;
}

bool cdbase::set_authkey(char authkey[SUPLA_AUTHKEY_SIZE]) {
  char _authkey[SUPLA_AUTHKEY_SIZE] = {};

  if (memcmp(_authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->authkey, authkey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);

  return true;
}

void cdbase::get_authkey(char authkey[SUPLA_AUTHKEY_SIZE]) {
  lck_lock(lck);
  memcpy(authkey, this->authkey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);
}

int cdbase::get_id(void) {
  int result = false;

  lck_lock(lck);
  result = ID;
  lck_unlock(lck);

  return result;
}

void cdbase::set_id(int ID) {
  lck_lock(lck);
  this->ID = ID;
  lck_unlock(lck);
}

void cdbase::set_user(supla_user *user) {
  lck_lock(lck);
  this->user = user;
  lck_unlock(lck);
}

supla_user *cdbase::get_user(void) {
  supla_user *result;

  lck_lock(lck);
  result = this->user;
  lck_unlock(lck);

  return result;
}

int cdbase::get_user_id(void) {
  supla_user *user = get_user();

  if (user != NULL) return user->getUserID();

  return 0;
}

supla_connection *cdbase::get_connection(void) { return conn; }

void cdbase::update_last_activity(void) {
  lck_lock(lck);
  gettimeofday(&last_activity_time, NULL);
  lck_unlock(lck);
}

int cdbase::get_activity_delay(void) {
  int result;
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  result = now.tv_sec - last_activity_time.tv_sec;
  lck_unlock(lck);

  return result;
}

unsigned char cdbase::get_protocol_version(void) {
  unsigned char result = 0;
  lck_lock(lck);
  if (conn) {
    result = conn->get_protocol_version();
  }
  lck_unlock(lck);
  return result;
}

cdbase *cdbase::retain_ptr(void) {
  lck_lock(lck);
  ptr_counter++;
  lck_unlock(lck);
  return this;
}

void cdbase::release_ptr(void) {
  lck_lock(lck);
  if (ptr_counter > 0) {
    ptr_counter--;
  }
  lck_unlock(lck);
}

bool cdbase::ptr_is_used(void) {
  bool result = false;
  lck_lock(lck);
  result = ptr_counter > 0;
  lck_unlock(lck);
  return result;
}

unsigned long cdbase::get_ptr_counter(void) {
  unsigned long result = 0;
  lck_lock(lck);
  result = ptr_counter;
  lck_unlock(lck);
  return result;
}

// static
int cdbase::get_authkey_cache_size(void) {
  return safe_array_count(cdbase::authkey_auth_cache_arr);
}

bool cdbase::authkey_auth(const char guid[SUPLA_GUID_SIZE],
                          const char email[SUPLA_EMAIL_MAXSIZE],
                          const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id,
                          database *db) {
  // The cache is designed to reduce the number of queries to the database, in
  // particular the number of calls to the function st_bcrypt_check, which
  // consumes CPU resources

  if (guid == NULL || authkey == NULL || email == NULL) {
    return false;
  }

  if (cdbase::authkey_auth_cache_size > 0) {
    safe_array_lock(cdbase::authkey_auth_cache_arr);

    for (int a = 0; a < safe_array_count(cdbase::authkey_auth_cache_arr); a++) {
      authkey_cache_item_t *i = static_cast<authkey_cache_item_t *>(
          safe_array_get(cdbase::authkey_auth_cache_arr, a));
      if (i && strncmp(i->email, email, SUPLA_EMAIL_MAXSIZE) == 0 &&
          memcmp(i->guid, guid, SUPLA_GUID_SIZE) == 0 &&
          memcmp(i->authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0) {
        bool result = i->result;
        *user_id = i->user_id;
        safe_array_move_to_begin(cdbase::authkey_auth_cache_arr, a);
        safe_array_unlock(cdbase::authkey_auth_cache_arr);
        return result;
      }
    }

    safe_array_unlock(cdbase::authkey_auth_cache_arr);

    authkey_cache_item_t *i = NULL;
    bool result = db_authkey_auth(guid, email, authkey, user_id, db);

    safe_array_lock(cdbase::authkey_auth_cache_arr);

    if (safe_array_count(cdbase::authkey_auth_cache_arr) >=
        cdbase::authkey_auth_cache_size) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      int count = safe_array_count(cdbase::authkey_auth_cache_arr);
      int idx = count / 2;
      if (idx > 0) {
        idx = count - 1 - (tv.tv_usec % idx);
      }

      i = static_cast<authkey_cache_item_t *>(
          safe_array_get(cdbase::authkey_auth_cache_arr, idx));

    } else {
      i = (authkey_cache_item_t *)malloc(sizeof(authkey_cache_item_t));
      memset(i, 0, sizeof(authkey_cache_item_t));
      safe_array_add(cdbase::authkey_auth_cache_arr, i);
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

    safe_array_unlock(cdbase::authkey_auth_cache_arr);
    return result;

  } else {
    return db_authkey_auth(guid, email, authkey, user_id, db);
  }
}

void cdbase::iterate() {}

unsigned _supla_int64_t cdbase::wait_time_usec() { return 120000000; }
