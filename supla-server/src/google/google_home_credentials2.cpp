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

#include "google/google_home_credentials2.h"

#include "db/db_access_provider.h"
#include "google/google_home_credentials_dao.h"
#include "http/httprequestqueue.h"
#include "log.h"

using std::list;

supla_google_home_credentials2::supla_google_home_credentials2(void)
    : supla_http_oauth_credentials() {}

supla_google_home_credentials2::supla_google_home_credentials2(supla_user *user)
    : supla_http_oauth_credentials(user) {}

supla_google_home_credentials2::~supla_google_home_credentials2(void) {}

void supla_google_home_credentials2::load() {
  supla_db_access_provider dba;
  supla_google_home_credentials_dao dao(&dba);

  set(dao.get_access_token(get_user_id()), "", 60 * 60 * 24 * 365);
  data_lock();
  excluded_channels.clear();
  data_unlock();
}

void supla_google_home_credentials2::exclude_channel(int channel_id) {
  data_lock();
  if (!is_channel_excluded(channel_id)) {
    excluded_channels.push_back(channel_id);
  }
  data_unlock();
}

bool supla_google_home_credentials2::is_channel_excluded(int channel_id) {
  bool result = false;

  data_lock();

  for (auto it = excluded_channels.begin(); it != excluded_channels.end();
       ++it) {
    if (*it == channel_id) {
      result = true;
      break;
    }
  }

  data_unlock();

  return result;
}
