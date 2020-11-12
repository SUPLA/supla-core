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

#include <mqtt_client_library_adapter.h>

supla_mqtt_client_library_adapter::supla_mqtt_client_library_adapter(
    supla_mqtt_client_settings *settings) {
  this->settings = settings;
  on_message_received_callback = NULL;
  on_connected_callback = NULL;
}
supla_mqtt_client_library_adapter::~supla_mqtt_client_library_adapter(void) {}

void supla_mqtt_client_library_adapter::set_on_message_received_callback(
    _on_message_received_cb cb) {
  on_message_received_callback = cb;
}

void supla_mqtt_client_library_adapter::set_on_connected_callback(
    _on_connected_cb cb) {
  on_connected_callback = cb;
}
