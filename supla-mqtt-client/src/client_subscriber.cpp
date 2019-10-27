/*
 * client_subscriber.cpp
 *
 *  Created on: 23 paź 2019
 *      Author: Łukasz Bek
 *   Copyright: Łukasz Bek
 */

#include "client_subscriber.h"

void handle_subscribed_message(void* supla_client,
                               supla_device_channels* channels,
                               client_config* config, std::string topic,
                               std::string message) {
  if (supla_client == NULL) return;
  if (channels == NULL) return;
  if (config == NULL) return;

  vector<client_command*> commands;
  config->getCommandsForTopic(topic, commands);

  if (commands.size() == 0) return;

  json payload;
  try {
    payload = json::parse(message);

  } catch (ser_error& exception) {
    supla_log(LOG_ERR,
              "error while parsing incoming message [topic: %s], [message:%s], "
              "[exception: %s]",
              topic.c_str(), message.c_str(), exception.what());
    return;
  }

  for (auto command : commands) {
    try {
      std::string idPath = command->getId();
      int id = jsonpointer::get(payload, idPath).as<int>();

      auto channel = channels->find_channel(id);
      if (channel == NULL) continue;

      switch (channel->getFunc()) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER: {
          std::string on_off = command->getOnOff();
          std::string value =
              jsonpointer::get(payload, on_off).as<std::string>();
          std::string on_value = command->getOnValue();

          if (value.compare(on_value) == 0)
            supla_client_open(supla_client, id, 0, 1);
          else
            supla_client_open(supla_client, id, 0, 0);
        } break;
      }
    } catch (json_exception& je) {
      supla_log(LOG_ERR,
                "error while trying get value from payload [error: %s]",
                je.what());
    } catch (exception& exception) {
      supla_log(LOG_ERR, "general error %s", exception.what());
    }
  }
}
