/*
 * client_publisher.h
 *
 *  Created on: 22 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#ifndef CLIENT_PUBLISHER_H_
#define CLIENT_PUBLISHER_H_

#include "client_config.h"
#include "client_state.h"
#include "devicechannel.h"
#include "mqtt/mqtt_client.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/proto.h"
#include "supla-client-lib/srpc.h"
#include "yaml/yaml.h"

void publish_mqtt_message_for_channel(client_config* config,
                                      supla_device_channel* channel);

#endif /* CLIENT_PUBLISHER_H_ */
