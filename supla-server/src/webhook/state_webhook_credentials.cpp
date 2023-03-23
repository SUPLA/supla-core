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

#include "state_webhook_credentials.h"

#include "db/db_access_provider.h"
#include "webhook/state_webhook_credentials_dao.h"

using std::string;
using std::vector;

supla_state_webhook_credentials::supla_state_webhook_credentials(void)
    : supla_http_oauth_credentials() {}

supla_state_webhook_credentials::supla_state_webhook_credentials(
    supla_user *user)
    : supla_http_oauth_credentials(user) {}

supla_state_webhook_credentials::~supla_state_webhook_credentials(void) {}

string supla_state_webhook_credentials::get_url(void) {
  data_lock();
  string result = url;
  data_unlock();

  return url;
}

vector<int> supla_state_webhook_credentials::get_function_ids(void) {
  data_lock();
  vector<int> result = function_ids;
  data_unlock();

  return result;
}

void supla_state_webhook_credentials::update(const string access_token,
                                             const string refresh_token,
                                             int expires_in) {
  supla_http_oauth_credentials::update(access_token, refresh_token, expires_in);

  supla_db_access_provider dba;
  supla_state_webhook_credentials_dao dao(&dba);
  dao.set(get_user_id(), access_token, refresh_token, expires_in);
}

void supla_state_webhook_credentials::remove(void) {
  data_lock();
  supla_http_oauth_credentials::remove();
  url = "";
  function_ids.clear();
  data_unlock();

  supla_db_access_provider dba;
  supla_state_webhook_credentials_dao dao(&dba);
  dao.remove(get_user_id());
}

void supla_state_webhook_credentials::load(void) {
  supla_db_access_provider dba;
  supla_state_webhook_credentials_dao dao(&dba);

  string access_token, refresh_token, url, functions;
  vector<int> function_ids;
  int expires_in = 0;

  dao.get(get_user_id(), &access_token, &refresh_token, &expires_in, &url,
          &functions);

  unsigned int last_pos = 0;
  int function = 0;
  char divider = 1;

  for (unsigned int a = 0; a < functions.size(); a++) {
    char c = functions.at(a);
    if (c >= '0' && c <= '9') {
      function = function * 10 + c - '0';
    } else if (c == '-' && a == last_pos) {
      divider = -1;
    } else {
      if (function > 0) {
        function_ids.push_back(function * divider);
      }

      last_pos = a + 1;
      function = 0;
      divider = 1;
    }
  }

  if (function > 0) {
    function_ids.push_back(function * divider);
  }

  data_lock();
  set(access_token, refresh_token, expires_in);
  this->url = url;
  this->function_ids = function_ids;
  data_unlock();
}
