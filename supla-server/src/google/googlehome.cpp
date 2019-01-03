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

#include "google/googlehome.h"
#include "database.h"

supla_google_home::supla_google_home(supla_user *user)
    : supla_voice_assistant_common(user) {}

int supla_google_home::get_token_maxsize(void) { return GH_TOKEN_MAXSIZE; }

void supla_google_home::load() {
  database *db = new database();

  if (!db->connect()) {
    set(NULL);
  }

  delete db;
}

void supla_google_home::on_credentials_changed() { load(); }
