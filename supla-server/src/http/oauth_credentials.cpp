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

#include "http/oauth_credentials.h"

#include "lck.h"
#include "user/user.h"

using std::string;

supla_http_oauth_credentials::supla_http_oauth_credentials(void) {
  refresh_lck = lck_init();
  data_lck = lck_init();

  this->user = nullptr;
  this->set_at = {};
  this->expires_at = {};
}

supla_http_oauth_credentials::supla_http_oauth_credentials(supla_user *user) {
  refresh_lck = lck_init();
  data_lck = lck_init();

  this->user = user;
  this->set_at = {};
  this->expires_at = {};
}

supla_http_oauth_credentials::~supla_http_oauth_credentials(void) {
  if (refresh_lck) {
    lck_free(refresh_lck);
  }

  if (data_lck) {
    lck_free(data_lck);
  }
}

void supla_http_oauth_credentials::refresh_lock(void) { lck_lock(refresh_lck); }

void supla_http_oauth_credentials::refresh_unlock(void) {
  lck_unlock(refresh_lck);
}

void supla_http_oauth_credentials::data_lock(void) { lck_lock(data_lck); }

void supla_http_oauth_credentials::data_unlock(void) { lck_unlock(data_lck); }

supla_user *supla_http_oauth_credentials::get_user(void) { return user; }

int supla_http_oauth_credentials::get_user_id(void) {
  if (user) {
    return user->getUserID();
  }
  return 0;
}

string supla_http_oauth_credentials::get_user_long_unique_id(void) {
  string result;
  if (user) {
    const char *id = user->getLongUniqueID();
    if (id) {
      result = id;
    }
  }
  return result;
}

string supla_http_oauth_credentials::get_user_short_unique_id(void) {
  string result;
  if (user) {
    const char *id = user->getShortUniqueID();
    if (id) {
      result = id;
    }
  }
  return result;
}

string supla_http_oauth_credentials::get_access_token(void) {
  data_lock();
  string result = access_token;
  data_unlock();
  return result;
}

string supla_http_oauth_credentials::get_refresh_token(void) {
  data_lock();
  string result = refresh_token;
  data_unlock();
  return result;
}

bool supla_http_oauth_credentials::is_access_token_exists(void) {
  data_lock();
  bool result = access_token.size();
  data_unlock();
  return result;
}

bool supla_http_oauth_credentials::is_refresh_token_exists(void) {
  data_lock();
  bool result = refresh_token.size();
  data_unlock();
  return result;
}

int supla_http_oauth_credentials::expires_in(void) {
  data_lock();

  struct timeval now;
  gettimeofday(&now, NULL);
  int result = expires_at.tv_sec - now.tv_sec;

  data_unlock();
  return result;
}

struct timeval supla_http_oauth_credentials::get_set_time(void) {
  data_lock();
  struct timeval result = set_at;
  data_unlock();

  return result;
}

void supla_http_oauth_credentials::update(const string access_token,
                                          const string refresh_token,
                                          int expires_in) {
  data_lock();

  this->access_token = access_token;
  this->refresh_token = refresh_token;

  if (expires_in == 0) {
    expires_in = 3600 * 24 * 365 * 10;
  }

  gettimeofday(&set_at, NULL);

  this->expires_at.tv_sec = set_at.tv_sec + expires_in;
  this->expires_at.tv_usec = set_at.tv_usec;

  data_unlock();
}

void supla_http_oauth_credentials::remove(void) {
  access_token = "";
  refresh_token = "";
  expires_at = {};
}
