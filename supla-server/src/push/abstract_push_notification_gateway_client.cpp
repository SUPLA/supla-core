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

#include "abstract_push_notification_gateway_client.h"

using std::string;

supla_abstract_push_notification_gateway_client::
    supla_abstract_push_notification_gateway_client(
        supla_abstract_curl_adapter *curl_adapter,
        supla_access_token_providers *token_providers,
        supla_push_notification *push) {
  this->curl_adapter = curl_adapter;
  this->token_providers = token_providers;
  this->push = push;
}

supla_abstract_push_notification_gateway_client::
    ~supla_abstract_push_notification_gateway_client(void) {}

supla_push_notification *
supla_abstract_push_notification_gateway_client::get_push_notification(void) {
  return push;
}

supla_abstract_curl_adapter *
supla_abstract_push_notification_gateway_client::get_curl_adapter(void) {
  return curl_adapter;
}

bool supla_abstract_push_notification_gateway_client::send(void) {
  bool any_sent = false;
  bool any_error = false;

  supla_abstract_access_token_provider *token_provider = nullptr;

  _app_id_e last_app_id = app_supla;

  for (size_t a = 0; a < push->get_recipients().count(get_platform()); a++) {
    supla_push_notification_recipient *recipient =
        push->get_recipients().get(get_platform(), a);

    if (!recipient || !recipient->is_exists()) {
      continue;
    }

    if (token_provider == nullptr || last_app_id != recipient->get_app_id()) {
      token_provider = token_providers->get_provider(get_platform(),
                                                     recipient->get_app_id());

      last_app_id = recipient->get_app_id();
    }

    if (token_provider) {
      string token = token_provider->get_token();
      string url = token_provider->get_url();

      curl_adapter->reset();

      if (token.empty() || url.empty() || !_send(url, token, recipient)) {
        any_error = true;
      } else {
        any_sent = true;
      }
    } else {
      any_error = true;
    }
  }

  return any_sent && !any_error;
}
