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

#include "alexa_credentials.h"

#include "amazon/alexa_credentials_dao.h"
#include "db/db_access_provider.h"

using std::string;

supla_amazon_alexa_credentials::supla_amazon_alexa_credentials(supla_user *user)
    : supla_http_oauth_credentials(user) {}

supla_amazon_alexa_credentials::supla_amazon_alexa_credentials()
    : supla_http_oauth_credentials() {}

supla_amazon_alexa_credentials::~supla_amazon_alexa_credentials() {}

void supla_amazon_alexa_credentials::remove(void) {
  data_lock();
  supla_http_oauth_credentials::remove();
  region = "";
  data_unlock();

  supla_db_access_provider dba;
  supla_amazon_alexa_credentials_dao dao(&dba);
  dao.remove(get_user_id());
}

string supla_amazon_alexa_credentials::get_region(void) {
  data_lock();
  string region = this->region;
  data_unlock();
  return region;
}

void supla_amazon_alexa_credentials::load(void) {
  supla_db_access_provider dba;
  supla_amazon_alexa_credentials_dao dao(&dba);

  string access_token, refresh_token, region;
  int expires_in = 0;

  if (dao.get(get_user_id(), &access_token, &refresh_token, &expires_in,
              &region)) {
    data_lock();
    set(access_token, refresh_token, expires_in);
    this->region = region;
    data_unlock();
  }
}

void supla_amazon_alexa_credentials::update(const string &access_token,
                                            const string &refresh_token,
                                            int expires_in) {
  supla_http_oauth_credentials::update(access_token, refresh_token, expires_in);

  supla_db_access_provider dba;
  supla_amazon_alexa_credentials_dao dao(&dba);
  dao.set(get_user_id(), access_token, refresh_token, expires_in);
}
