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

#include "pn_recipient.h"

using std::string;

supla_pn_recipient::supla_pn_recipient(int client_id, int app_id,
                                       bool development_env,
                                       const string& token) {
  this->client_id = client_id;
  this->token = token;
  this->app_id = app_id;
  this->development_env = development_env;
  this->exists = true;
}

supla_pn_recipient::supla_pn_recipient(supla_pn_recipient* recipient) {
  this->client_id = recipient->client_id;
  this->token = recipient->token;
  this->app_id = recipient->app_id;
  this->development_env = recipient->development_env;
  this->exists = recipient->exists;
}

supla_pn_recipient::~supla_pn_recipient(void) {}

const std::string& supla_pn_recipient::get_token(void) { return token; }

int supla_pn_recipient::get_client_id(void) { return client_id; }

int supla_pn_recipient::get_app_id(void) { return app_id; }

void supla_pn_recipient::set_message_id(const std::string& message_id) {
  this->message_id = message_id;
}

std::string supla_pn_recipient::get_message_id(void) { return message_id; }

bool supla_pn_recipient::is_exists(void) { return exists; }

bool supla_pn_recipient::is_development_env(void) { return development_env; }

void supla_pn_recipient::set_exists(bool exists) { this->exists = exists; }
