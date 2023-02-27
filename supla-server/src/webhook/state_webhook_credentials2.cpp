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

#include "webhook/state_webhook_credentials2.h"

using std::list;
using std::string;

#define WEBHOOK_TOKEN_MAXSIZE 255
#define WEBHOOK_URL_MAXSIZE 255
#define WEBHOOK_FUNCTIONS_IDS_MAXSIZE 255

supla_state_webhook_credentials2::supla_state_webhook_credentials2(void)
    : supla_http_oauth_credentials() {}

supla_state_webhook_credentials2::supla_state_webhook_credentials2(
    supla_user *user)
    : supla_http_oauth_credentials(user) {}

supla_state_webhook_credentials2::~supla_state_webhook_credentials2(void) {}

string supla_state_webhook_credentials2::get_url(void) {
  data_lock();
  string result = url;
  data_unlock();

  return url;
}

list<int> supla_state_webhook_credentials2::get_function_ids(void) {
  data_lock();
  list<int> result = functions_ids;
  data_unlock();

  return result;
}

void supla_state_webhook_credentials2::update(const std::string access_token,
                                              const std::string refresh_token,
                                              int expires_in) {
  supla_http_oauth_credentials::update(access_token, refresh_token, expires_in);
}

void supla_state_webhook_credentials2::remove(void) {
  supla_http_oauth_credentials::remove();
}

void supla_state_webhook_credentials2::load(void) {}
