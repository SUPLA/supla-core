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

#include "apns_client.h"

using std::string;

supla_apns_client::supla_apns_client(
    supla_abstract_curl_adapter *curl_adapter,
    supla_access_token_providers *token_providers,
    supla_push_notification *push)
    : supla_abstract_push_notification_gateway_client(curl_adapter,
                                                      token_providers, push) {}

supla_apns_client::~supla_apns_client(void) {}

_platform_e supla_apns_client::get_platform(void) { return platform_ios; }

bool supla_apns_client::_send(const string &url, const string &token,
                              supla_pn_recipient *recipient) {
  return false;
}
