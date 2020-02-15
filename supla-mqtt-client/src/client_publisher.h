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

#ifndef CLIENT_PUBLISHER_H_
#define CLIENT_PUBLISHER_H_

#include "client_config.h"
#include "client_device.h"
#include "client_state.h"
#include "mqtt/mqtt_client.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/proto.h"
#include "supla-client-lib/srpc.h"
#include "yaml/yaml.h"

void publish_mqtt_message_for_event(client_config *config,
                                    TSC_SuplaEvent *event);

void publish_mqtt_message_for_channel(client_device_channel *channel);

#endif /* CLIENT_PUBLISHER_H_ */
