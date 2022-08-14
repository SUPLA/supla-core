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

#include "conn/connection_object.h"

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

void *supla_connection_object::authkey_auth_cache_arr = NULL;
int supla_connection_object::authkey_auth_cache_size = 0;

// static
void supla_connection_object::init(void) {
  supla_connection_object::authkey_auth_cache_size =
      scfg_int(CFG_LIMIT_AUTHKEY_AUTH_CACHE_SIZE);
  supla_connection_object::authkey_auth_cache_arr = safe_array_init();
}

// static
void supla_connection_object::release_cache(void) {
  for (int a = 0;
       a < safe_array_count(supla_connection_object::authkey_auth_cache_arr);
       a++) {
    authkey_cache_item_t *i = static_cast<authkey_cache_item_t *>(
        safe_array_get(supla_connection_object::authkey_auth_cache_arr, a));
    if (i) {
      if (i->email) {
        free(i->email);
      }
      free(i);
    }
  }

  safe_array_free(supla_connection_object::authkey_auth_cache_arr);
}

supla_connection_object::supla_connection_object(supla_connection *conn) {
  this->user = NULL;
  this->conn = conn;
  this->lck = lck_init();
  this->ID = 0;
  this->ptr_counter = 0;
  memset(this->guid, 0, SUPLA_GUID_SIZE);
  memset(this->authkey, 0, SUPLA_AUTHKEY_SIZE);

  update_last_activity();  // last line / after lck_init
}

supla_connection_object::~supla_connection_object() { lck_free(this->lck); }

void supla_connection_object::terminate(void) {
  lck_lock(lck);
  if (conn) conn->terminate();
  lck_unlock(lck);
}

bool supla_connection_object::set_guid(char guid[SUPLA_GUID_SIZE]) {
  char _guid[SUPLA_GUID_SIZE] = {};

  if (memcmp(_guid, guid, SUPLA_GUID_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->guid, guid, SUPLA_GUID_SIZE);
  lck_unlock(lck);

  return true;
}

void supla_connection_object::get_guid(char guid[SUPLA_GUID_SIZE]) {
  lck_lock(lck);
  memcpy(guid, this->guid, SUPLA_GUID_SIZE);
  lck_unlock(lck);
}

bool supla_connection_object::cmp_guid(const char guid[SUPLA_GUID_SIZE]) {
  bool result = false;

  lck_lock(lck);
  result = memcmp(this->guid, guid, SUPLA_GUID_SIZE) == 0;
  lck_unlock(lck);

  return result;
}

bool supla_connection_object::set_authkey(char authkey[SUPLA_AUTHKEY_SIZE]) {
  char _authkey[SUPLA_AUTHKEY_SIZE] = {};

  if (memcmp(_authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->authkey, authkey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);

  return true;
}

void supla_connection_object::get_authkey(char authkey[SUPLA_AUTHKEY_SIZE]) {
  lck_lock(lck);
  memcpy(authkey, this->authkey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);
}

int supla_connection_object::get_id(void) {
  int result = false;

  lck_lock(lck);
  result = ID;
  lck_unlock(lck);

  return result;
}

void supla_connection_object::set_id(int ID) {
  lck_lock(lck);
  this->ID = ID;
  lck_unlock(lck);
}

void supla_connection_object::set_user(supla_user *user) {
  lck_lock(lck);
  this->user = user;
  lck_unlock(lck);
}

supla_user *supla_connection_object::get_user(void) {
  supla_user *result;

  lck_lock(lck);
  result = this->user;
  lck_unlock(lck);

  return result;
}

int supla_connection_object::get_user_id(void) {
  supla_user *user = get_user();

  if (user != NULL) return user->getUserID();

  return 0;
}

supla_connection *supla_connection_object::get_connection(void) { return conn; }

void supla_connection_object::update_last_activity(void) {
  lck_lock(lck);
  gettimeofday(&last_activity_time, NULL);
  lck_unlock(lck);
}

int supla_connection_object::get_activity_delay(void) {
  int result;
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  result = now.tv_sec - last_activity_time.tv_sec;
  lck_unlock(lck);

  return result;
}

unsigned char supla_connection_object::get_protocol_version(void) {
  unsigned char result = 0;
  lck_lock(lck);
  if (conn) {
    result = conn->get_protocol_version();
  }
  lck_unlock(lck);
  return result;
}

supla_connection_object *supla_connection_object::retain_ptr(void) {
  lck_lock(lck);
  ptr_counter++;
  lck_unlock(lck);
  return this;
}

void supla_connection_object::release_ptr(void) {
  lck_lock(lck);
  if (ptr_counter > 0) {
    ptr_counter--;
  }
  lck_unlock(lck);
}

bool supla_connection_object::ptr_is_used(void) {
  bool result = false;
  lck_lock(lck);
  result = ptr_counter > 0;
  lck_unlock(lck);
  return result;
}

unsigned long supla_connection_object::get_ptr_counter(void) {
  unsigned long result = 0;
  lck_lock(lck);
  result = ptr_counter;
  lck_unlock(lck);
  return result;
}

// static
int supla_connection_object::get_authkey_cache_size(void) {
  return safe_array_count(supla_connection_object::authkey_auth_cache_arr);
}

bool supla_connection_object::authkey_auth(
    const char guid[SUPLA_GUID_SIZE], const char email[SUPLA_EMAIL_MAXSIZE],
    const char authkey[SUPLA_AUTHKEY_SIZE], int *user_id, database *db) {
  // The cache is designed to reduce the number of queries to the database, in
  // particular the number of calls to the function st_bcrypt_check, which
  // consumes CPU resources

  if (guid == NULL || authkey == NULL || email == NULL) {
    return false;
  }

  if (supla_connection_object::authkey_auth_cache_size > 0) {
    safe_array_lock(supla_connection_object::authkey_auth_cache_arr);

    for (int a = 0;
         a < safe_array_count(supla_connection_object::authkey_auth_cache_arr);
         a++) {
      authkey_cache_item_t *i = static_cast<authkey_cache_item_t *>(
          safe_array_get(supla_connection_object::authkey_auth_cache_arr, a));
      if (i && strncmp(i->email, email, SUPLA_EMAIL_MAXSIZE) == 0 &&
          memcmp(i->guid, guid, SUPLA_GUID_SIZE) == 0 &&
          memcmp(i->authkey, authkey, SUPLA_AUTHKEY_SIZE) == 0) {
        bool result = i->result;
        *user_id = i->user_id;
        safe_array_move_to_begin(
            supla_connection_object::authkey_auth_cache_arr, a);
        safe_array_unlock(supla_connection_object::authkey_auth_cache_arr);
        return result;
      }
    }

    safe_array_unlock(supla_connection_object::authkey_auth_cache_arr);

    authkey_cache_item_t *i = NULL;
    bool result = db_authkey_auth(guid, email, authkey, user_id, db);

    safe_array_lock(supla_connection_object::authkey_auth_cache_arr);

    if (safe_array_count(supla_connection_object::authkey_auth_cache_arr) >=
        supla_connection_object::authkey_auth_cache_size) {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      int count =
          safe_array_count(supla_connection_object::authkey_auth_cache_arr);
      int idx = count / 2;
      if (idx > 0) {
        idx = count - 1 - (tv.tv_usec % idx);
      }

      i = static_cast<authkey_cache_item_t *>(
          safe_array_get(supla_connection_object::authkey_auth_cache_arr, idx));

    } else {
      i = (authkey_cache_item_t *)malloc(sizeof(authkey_cache_item_t));
      memset(i, 0, sizeof(authkey_cache_item_t));
      safe_array_add(supla_connection_object::authkey_auth_cache_arr, i);
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

    safe_array_unlock(supla_connection_object::authkey_auth_cache_arr);
    return result;

  } else {
    return db_authkey_auth(guid, email, authkey, user_id, db);
  }
}

void supla_connection_object::iterate() {}

unsigned _supla_int64_t supla_connection_object::wait_time_usec() {
  return 120000000;
}
