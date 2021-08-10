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

#include <google/googlehomecredentials.h>
#include "database.h"
#include "http/httprequestqueue.h"
#include "log.h"
#include "user/user.h"

supla_google_home_credentials::supla_google_home_credentials(supla_user *user)
    : supla_webhook_basic_credentials(user) {
  sync_40x_counter = 0;
}

int supla_google_home_credentials::get_token_maxsize(void) {
  return GH_TOKEN_MAXSIZE;
}

void supla_google_home_credentials::load() {
  database *db = new database();

  if (!db->connect() || !db->google_home_load_credentials(this)) {
    set(NULL);
  }

  delete db;
}

void supla_google_home_credentials::on_credentials_changed() { load(); }

void supla_google_home_credentials::on_sync_40x_error() {
  bool _set_null = false;

  data_lock();

  sync_40x_counter++;
  if (sync_40x_counter >= 2) {
    sync_40x_counter = 0;
    _set_null = true;
  }

  data_unlock();

  if (_set_null) {
    supla_log(LOG_INFO,
              "Communication with the HomeGraph bridge paused for the user: %i",
              getUser()->getUserID());
    set(NULL);
  }
}

void supla_google_home_credentials::on_reportstate_404_error() {
  supla_http_request_queue::getInstance()->onGoogleHomeSyncNeededEvent(
      getUser(), EST_GOOGLE_HOME);
}

void supla_google_home_credentials::update(const char *access_token,
                                           const char *refresh_token,
                                           int expires_in) {}
