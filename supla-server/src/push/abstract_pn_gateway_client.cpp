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

#include "abstract_pn_gateway_client.h"

using std::string;
using std::vector;

supla_abstract_pn_gateway_client::supla_abstract_pn_gateway_client(
    supla_abstract_curl_adapter *curl_adapter,
    supla_pn_gateway_access_token_provider *token_provider,
    supla_push_notification *push) {
  this->curl_adapter = curl_adapter;
  this->token_provider = token_provider;
  this->push = push;
}

supla_abstract_pn_gateway_client::~supla_abstract_pn_gateway_client(void) {}

supla_push_notification *
supla_abstract_pn_gateway_client::get_push_notification(void) {
  return push;
}

supla_abstract_curl_adapter *supla_abstract_pn_gateway_client::get_curl_adapter(
    void) {
  return curl_adapter;
}

void supla_abstract_pn_gateway_client::add_args(const vector<string> &args,
                                                const string &key_name,
                                                cJSON *parent) {
  cJSON *arr = cJSON_AddArrayToObject(parent, key_name.c_str());

  for (auto it = args.begin(); it != args.end(); ++it) {
    cJSON_AddItemToArray(arr, cJSON_CreateString(it->c_str()));
  }
}

bool supla_abstract_pn_gateway_client::send(void) {
  bool any_sent = false;
  bool any_error = false;

  for (size_t a = 0; a < push->get_recipients().count(get_platform()); a++) {
    supla_pn_recipient *recipient =
        push->get_recipients().get(get_platform(), a);

    if (!recipient || !recipient->is_exists()) {
      continue;
    }

    supla_pn_gateway_access_token token =
        token_provider->get_token(get_platform(), recipient->get_app_id());

    curl_adapter->reset();

    if (!token.is_valid() || !_send(&token, recipient)) {
      any_error = true;
    } else {
      any_sent = true;
    }
  }

  return any_sent && !any_error;
}
