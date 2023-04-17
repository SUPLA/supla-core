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

#include "fcm_client.h"

supla_fcm_client::supla_fcm_client(
    supla_abstract_curl_adapter *curl_adapter,
    supla_abstract_access_token_provider *token_provider,
    supla_push_notification *push) {
  this->curl_adapter = curl_adapter;
  this->token_provider = token_provider;
  this->push = push;
}

supla_fcm_client::~supla_fcm_client(void) {}

bool supla_fcm_client::send(void) { return false; }
