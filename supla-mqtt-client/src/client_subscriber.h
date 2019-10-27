/*
 * client_subscriber.h
 *
 *  Created on: 23 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#ifndef CLIENT_SUBSCRIBER_H_
#define CLIENT_SUBSCRIBER_H_

#include <string>
#include "client_config.h"
#include "devicechannel.h"
#include "json/jsoncons/json.hpp"
#include "json/jsoncons_ext/jsonpointer/jsonpointer.hpp"
#include "supla-client-lib/log.h"
#include "supla-client-lib/supla-client.h"

using namespace jsoncons;

void handle_subscribed_message(void* supla_client,
                               supla_device_channels* channels,
                               client_config* config, std::string topic,
                               std::string message);

#endif /* CLIENT_SUBSCRIBER_H_ */
