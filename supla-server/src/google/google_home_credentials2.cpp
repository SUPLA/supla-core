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

supla_google_home_credentials2::supla_google_home_credentials2(void)
    : supla_http_oauth_credentials() {
  sync_40x_counter = 0;
}

supla_google_home_credentials2::supla_google_home_credentials2(supla_user *user)
    : supla_http_oauth_credentials(user) {
  sync_40x_counter = 0;
}

supla_google_home_credentials2::~supla_google_home_credentials2(void) {}

void supla_google_home_credentials2::on_sync_40x_error() {
  // For some unknown reason, home graph sometimes returns 404 when calling
  // action.devices.SYNC, but sync works fine. Therefore, we do not react to
  // this code for now.

  /*
    bool disable = false;

    data_lock();

    sync_40x_counter++;
    if (sync_40x_counter >= 2) {
      sync_40x_counter = 0;
      disable = true;
    }

    data_unlock();

    if (disable) {
      supla_log(LOG_INFO,
                "Communication with the HomeGraph bridge paused for the user:
    %i", get_user_id());

      set("", "", 0);
    }
    */
}

void supla_google_home_credentials2::on_reportstate_404_error() {
  // We're not pushing re-syncing because currently, for some reason, a 404 code
  // isn't always diagnostic.

  /*
    supla_http_request_queue::getInstance()->onGoogleHomeSyncNeededEvent(
        get_user(), supla_caller(ctGoogleHome));
  */
}

void supla_google_home_credentials2::load() {
  supla_db_access_provider dba;
  supla_google_home_credentials_dao dao(&dba);

  set(dao.get_access_token(get_user_id()), "", 60 * 60 * 24 * 365);
}
