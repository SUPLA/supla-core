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

#include "pn_gateway_access_token.h"

#include <sys/time.h>

using std::string;

supla_pn_gateway_access_token::supla_pn_gateway_access_token(void) {
  gettimeofday(&expires_at, nullptr);
  expires_at.tv_sec += 60 * 60 * 24 * 365 * 10;  // 10 years

  platform = platform_unknown;
  app_id = app_supla;
}

supla_pn_gateway_access_token::supla_pn_gateway_access_token(
    const string &url, const string &token, int expires_in,
    _platform_e platform, _app_id_e app_id) {
  gettimeofday(&expires_at, nullptr);
  if (expires_in == 0) {
    expires_in += 60 * 60 * 24 * 365 * 10;  // 10 years
  }
  expires_at.tv_sec += expires_in;

  this->platform = platform;
  this->app_id = app_id;
  this->url = url;
  this->token = token;
}

supla_pn_gateway_access_token::~supla_pn_gateway_access_token(void) {}

_platform_e supla_pn_gateway_access_token::get_platform(void) {
  return platform;
}

_app_id_e supla_pn_gateway_access_token::get_app_id(void) { return app_id; }

string supla_pn_gateway_access_token::get_url(void) { return url; }

string supla_pn_gateway_access_token::get_token(void) { return token; }

void supla_pn_gateway_access_token::set_token(const string &token) {
  this->token = token;
}

bool supla_pn_gateway_access_token::is_valid(void) {
  if (token.size()) {
    struct timeval now = {};
    gettimeofday(&now, nullptr);
    return now.tv_sec < expires_at.tv_sec;
  }

  return false;
}

int supla_pn_gateway_access_token::get_expires_in(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  return expires_at.tv_sec - now.tv_sec;
}

void supla_pn_gateway_access_token::get_expiration_time_if_earlier(
    struct timeval *expires_at) {
  if (expires_at->tv_sec == 0 || expires_at->tv_sec > this->expires_at.tv_sec ||
      (expires_at->tv_sec == this->expires_at.tv_sec &&
       expires_at->tv_usec > this->expires_at.tv_usec)) {
    *expires_at = this->expires_at;
  }
}
