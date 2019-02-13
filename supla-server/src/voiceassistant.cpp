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
#include <voiceassistant.h>
#include "database.h"
#include "lck.h"
#include "log.h"
#include "user.h"

supla_voice_assistant::supla_voice_assistant(supla_user *user) {
  lck1 = lck_init();
  lck2 = lck_init();

  this->user = user;
  this->access_token = NULL;
  this->set_at.tv_sec = 0;
  this->set_at.tv_usec = 0;

  set(NULL);
}

void supla_voice_assistant::token_free(void) {
  if (this->access_token) {
    free(this->access_token);
    this->access_token = NULL;
  }
}

supla_voice_assistant::~supla_voice_assistant() {
  if (lck1) {
    lck_free(lck1);
    lck1 = NULL;
  }

  if (lck2) {
    lck_free(lck2);
    lck2 = NULL;
  }

  token_free();
}

int supla_voice_assistant::getUserID() { return user->getUserID(); }

supla_user *supla_voice_assistant::getUser() { return user; }

void supla_voice_assistant::data_lock(void) { lck_lock(lck1); }

void supla_voice_assistant::data_unlock(void) { lck_unlock(lck1); }

void supla_voice_assistant::refresh_lock(void) { lck_lock(lck2); }

void supla_voice_assistant::refresh_unlock(void) { lck_unlock(lck2); }

bool supla_voice_assistant::isAccessTokenExists(void) {
  bool result = false;

  data_lock();
  result = access_token != NULL;
  data_unlock();

  return result;
}

char *supla_voice_assistant::getAccessToken(void) {
  char *result = NULL;

  data_lock();

  if (access_token != NULL) {
    result = strndup(access_token, get_token_maxsize());
  }

  data_unlock();

  return result;
}

struct timeval supla_voice_assistant::getSetTime(void) {
  struct timeval result;
  result.tv_sec = 0;
  result.tv_usec = 0;

  data_lock();
  result = set_at;
  data_unlock();

  return result;
}

void supla_voice_assistant::set(const char *access_token) {
  data_lock();
  token_free();

  int token_len = access_token ? strnlen(access_token, get_token_maxsize()) : 0;
  if (token_len > 0) {
    this->access_token = strndup(access_token, token_len);
  }

  gettimeofday(&set_at, NULL);

  data_unlock();
}
