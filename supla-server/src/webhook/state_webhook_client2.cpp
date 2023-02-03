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

#include <webhook/state_webhook_client2.h>

supla_state_webhook_client2::supla_state_webhook_client2(
    int channel_id, bool connected, supla_abstract_curl_adapter *curl_adapter) {
  this->channel_id = channel_id;
  this->connected = connected;
  this->curl_adapter = curl_adapter;
}

bool supla_state_webhook_client2::power_switch_report(bool on) {}

bool supla_state_webhook_client2::light_switch_report(bool on) {}
