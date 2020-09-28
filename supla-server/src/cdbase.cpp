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
  char GUID[SUPLA_GUID_SIZE];
  char *Email;
  char AuthKey[SUPLA_AUTHKEY_SIZE];
  int UserID;
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
      if (i->Email) {
        free(i->Email);
      }
      free(i);
    }
  }

  safe_array_free(cdbase::authkey_auth_cache_arr);
}

cdbase::cdbase(serverconnection *svrconn) {
  this->user = NULL;
  this->svrconn = svrconn;
  this->lck = lck_init();
  this->ID = 0;
  this->ptr_counter = 0;
  memset(this->GUID, 0, SUPLA_GUID_SIZE);
  memset(this->AuthKey, 0, SUPLA_AUTHKEY_SIZE);

  updateLastActivity();  // last line / after lck_init
}

cdbase::~cdbase() { lck_free(this->lck); }

void cdbase::terminate(void) {
  lck_lock(lck);
  if (svrconn) svrconn->terminate();
  lck_unlock(lck);
}

bool cdbase::setGUID(char GUID[SUPLA_GUID_SIZE]) {
  char _GUID[SUPLA_GUID_SIZE];
  memset(_GUID, 0, SUPLA_GUID_SIZE);

  if (memcmp(_GUID, GUID, SUPLA_GUID_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->GUID, GUID, SUPLA_GUID_SIZE);
  lck_unlock(lck);

  return true;
}

void cdbase::getGUID(char GUID[SUPLA_GUID_SIZE]) {
  lck_lock(lck);
  memcpy(GUID, this->GUID, SUPLA_GUID_SIZE);
  lck_unlock(lck);
}

bool cdbase::cmpGUID(const char GUID[SUPLA_GUID_SIZE]) {
  bool result = false;

  lck_lock(lck);
  result = memcmp(this->GUID, GUID, SUPLA_GUID_SIZE) == 0;
  lck_unlock(lck);

  return result;
}

bool cdbase::setAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  char _AuthKey[SUPLA_AUTHKEY_SIZE];
  memset(_AuthKey, 0, SUPLA_AUTHKEY_SIZE);

  if (memcmp(_AuthKey, AuthKey, SUPLA_AUTHKEY_SIZE) == 0) return false;

  lck_lock(lck);
  memcpy(this->AuthKey, AuthKey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);

  return true;
}

void cdbase::getAuthKey(char AuthKey[SUPLA_AUTHKEY_SIZE]) {
  lck_lock(lck);
  memcpy(AuthKey, this->AuthKey, SUPLA_AUTHKEY_SIZE);
  lck_unlock(lck);
}

int cdbase::getID(void) {
  int result = false;

  lck_lock(lck);
  result = ID;
  lck_unlock(lck);

  return result;
}

void cdbase::setID(int ID) {
  lck_lock(lck);
  this->ID = ID;
  lck_unlock(lck);
}

void cdbase::setUser(supla_user *user) {
  lck_lock(lck);
  this->user = user;
  lck_unlock(lck);
}

supla_user *cdbase::getUser(void) {
  supla_user *result;

  lck_lock(lck);
  result = this->user;
  lck_unlock(lck);

  return result;
}

int cdbase::getUserID(void) {
  supla_user *user = getUser();

  if (user != NULL) return user->getUserID();

  return 0;
}

serverconnection *cdbase::getSvrConn(void) { return svrconn; }

void cdbase::updateLastActivity(void) {
  lck_lock(lck);
  gettimeofday(&last_activity_time, NULL);
  lck_unlock(lck);
}

int cdbase::getActivityDelay(void) {
  int result;
  struct timeval now;
  gettimeofday(&now, NULL);

  lck_lock(lck);
  result = now.tv_sec - last_activity_time.tv_sec;
  lck_unlock(lck);

  return result;
}

unsigned char cdbase::getProtocolVersion(void) {
  unsigned char result = 0;
  lck_lock(lck);
  if (svrconn) {
    result = svrconn->getProtocolVersion();
  }
  lck_unlock(lck);
  return result;
}

cdbase *cdbase::retainPtr(void) {
  lck_lock(lck);
  ptr_counter++;
  lck_unlock(lck);
  return this;
}

void cdbase::releasePtr(void) {
  lck_lock(lck);
  if (ptr_counter > 0) {
    ptr_counter--;
  }
  lck_unlock(lck);
}

bool cdbase::ptrIsUsed(void) {
  bool result = false;
  lck_lock(lck);
  result = ptr_counter > 0;
  lck_unlock(lck);
  return result;
}

unsigned long cdbase::ptrCounter(void) {
  unsigned long result = 0;
  lck_lock(lck);
  result = ptr_counter;
  lck_unlock(lck);
  return result;
}

// static
int cdbase::getAuthKeyCacheSize(void) {
  return safe_array_count(cdbase::authkey_auth_cache_arr);
}

bool cdbase::authkey_auth(const char GUID[SUPLA_GUID_SIZE],
                          const char Email[SUPLA_EMAIL_MAXSIZE],
                          const char AuthKey[SUPLA_AUTHKEY_SIZE], int *UserID,
                          database *db) {
  // The cache is designed to reduce the number of queries to the database, in
  // particular the number of calls to the function st_bcrypt_check, which
  // consumes CPU resources

  if (GUID == NULL || AuthKey == NULL || Email == NULL) {
    return false;
  }

  if (cdbase::authkey_auth_cache_size > 0) {
    safe_array_lock(cdbase::authkey_auth_cache_arr);

    for (int a = 0; a < safe_array_count(cdbase::authkey_auth_cache_arr); a++) {
      authkey_cache_item_t *i = static_cast<authkey_cache_item_t *>(
          safe_array_get(cdbase::authkey_auth_cache_arr, a));
      if (i && strncmp(i->Email, Email, SUPLA_EMAIL_MAXSIZE) == 0 &&
          memcmp(i->GUID, GUID, SUPLA_GUID_SIZE) == 0 &&
          memcmp(i->AuthKey, AuthKey, SUPLA_AUTHKEY_SIZE) == 0) {
        bool result = i->result;
        *UserID = i->UserID;
        safe_array_move_to_begin(cdbase::authkey_auth_cache_arr, a);
        safe_array_unlock(cdbase::authkey_auth_cache_arr);
        return result;
      }
    }

    safe_array_unlock(cdbase::authkey_auth_cache_arr);

    authkey_cache_item_t *i = NULL;
    bool result = db_authkey_auth(GUID, Email, AuthKey, UserID, db);

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
      memcpy(i->GUID, GUID, SUPLA_GUID_SIZE);
      memcpy(i->AuthKey, AuthKey, SUPLA_AUTHKEY_SIZE);

      if (i->Email == NULL) {
        i->Email = strndup(Email, SUPLA_EMAIL_MAXSIZE);
      } else {
        int len1 = strnlen(Email, SUPLA_EMAIL_MAXSIZE);
        int len2 = strnlen(i->Email, SUPLA_EMAIL_MAXSIZE);
        if (len1 > len2) {
          i->Email = (char *)realloc(i->Email, len1 + 1);
        }
        snprintf(i->Email, len1 + 1, "%s", Email);
      }

      i->result = result;
      i->UserID = *UserID;
    }

    safe_array_unlock(cdbase::authkey_auth_cache_arr);
    return result;

  } else {
    return db_authkey_auth(GUID, Email, AuthKey, UserID, db);
  }
}

void cdbase::iterate() {}

unsigned _supla_int64_t cdbase::waitTimeUSec() { return 120000000; }
