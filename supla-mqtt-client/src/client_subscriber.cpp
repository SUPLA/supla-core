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

#include "client_subscriber.h"
#include <vector>

bool value_exists(jsoncons::json payload, std::string path) {
  try {
    return jsoncons::jsonpointer::contains(payload, path);
  } catch (jsoncons::json_exception& je) {
    return false;
  }
}

void handle_subscribed_message(void* supla_client,
                               client_device_channels* channels,
                               client_config* config, std::string topic,
                               std::string message) {
  supla_log(LOG_DEBUG, "handling message %s", message);

  if (supla_client == NULL) return;
  if (channels == NULL) return;
  if (config == NULL) return;

  std::vector<client_command*> commands;
  config->getCommandsForTopic(topic, &commands);

  if (commands.size() == 0) return;

  jsoncons::json payload;
  try {
    payload = jsoncons::json::parse(message);
  } catch (jsoncons::ser_error& exception) {
    supla_log(LOG_ERR,
              "error while parsing incoming message [topic: %s], [message:%s], "
              "[exception: %s]",
              topic.c_str(), message.c_str(), exception.what());
    return;
  }

  supla_log(LOG_DEBUG, "handling incomming message: %s", message.c_str());

  for (auto command : commands) {
    try {
      std::string idPath = command->getId();
      int id = jsoncons::jsonpointer::get(payload, idPath).as<int>();

      auto channel = channels->find_channel(id);
      if (channel == NULL) continue;

      supla_log(LOG_DEBUG, "found command for topic id: %d  func: %d", id,
                channel->getFunc());

      switch (channel->getFunc()) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER: {
          supla_log(LOG_DEBUG,
                    "incomming message is for channel one of"
                    "[POWERSWITCH, LIGHTSWITCH, STAIRCASETIMER]");

          std::string on_off = command->getOnOff();
          std::string value =
              jsoncons::jsonpointer::get(payload, on_off).as<std::string>();
          std::string on_value = command->getOnValue();

          supla_log(LOG_DEBUG, "handling %d %s %s", channel->getFunc(), value,
                    on_value);

          if (value.compare(on_value) == 0) {
            supla_log(LOG_DEBUG, "supla_client_open(id: %d, state: %d", id, 1);
            supla_client_open(supla_client, id, 0, 1);
          } else {
            supla_log(LOG_DEBUG, "supla_client_open(id: %d, state: %d", id, 0);
            supla_client_open(supla_client, id, 0, 0);
          }
          return;
        } break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER: {
          std::string shut = command->getShut();

          unsigned char value =
              jsoncons::jsonpointer::get(payload, shut).as<unsigned char>();

          value += 10;

          supla_log(LOG_DEBUG, "supla_client_open(id: %d, state: %d", id,
                    value);
          supla_client_open(supla_client, id, 0, value);
        } break;
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE: {
          std::string hi = command->getShut();

          unsigned char value =
              jsoncons::jsonpointer::get(payload, hi).as<unsigned char>();

          supla_log(LOG_DEBUG, "supla_client_open(id: %d, state: %d", id,
                    value);
          supla_client_open(supla_client, id, 0, value);
        } break;
        case SUPLA_CHANNELFNC_DIMMER:
        case SUPLA_CHANNELFNC_RGBLIGHTING:
        case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
          std::string brightness = command->getBrightness();
          std::string color_brightness = command->getColorBrghtness();
          std::string color_r = command->getColorR();
          std::string color_g = command->getColorG();
          std::string color_b = command->getColorB();
          std::string color = command->getColor();

          char brightness_value = 0;
          char color_brightness_value = 0;
          int color_int;
          char on_off;
          bool isRgbChannelCommand = false;

          channel->getRGBW(&color_int, &color_brightness_value,
                           &brightness_value, &on_off);

          unsigned char color_b_value =
              (unsigned char)((color_int & 0x000000FF));
          unsigned char color_g_value =
              (unsigned char)((color_int & 0x0000FF00) >> 8);
          unsigned char color_r_value =
              (unsigned char)((color_int & 0x00FF0000) >> 16);

          if (brightness.length() > 0 && (value_exists(payload, brightness))) {
            brightness_value =
                jsoncons::jsonpointer::get(payload, brightness).as<char>();
            isRgbChannelCommand = true;
          }

          if (color_brightness.length() > 0 &&
              (value_exists(payload, color_brightness))) {
            color_brightness_value =
                jsoncons::jsonpointer::get(payload, color_brightness)
                    .as<char>();
            isRgbChannelCommand = true;
          }

          if (color_r.length() > 0 && (value_exists(payload, color_r))) {
            color_r_value =
                jsoncons::jsonpointer::get(payload, color_r).as<uint8_t>();
            isRgbChannelCommand = true;
          }

          if (color_g.length() > 0 && (value_exists(payload, color_g))) {
            color_g_value =
                jsoncons::jsonpointer::get(payload, color_g).as<uint8_t>();
            isRgbChannelCommand = true;
          }

          if (color_b.length() > 0 && (value_exists(payload, color_b))) {
            color_b_value =
                jsoncons::jsonpointer::get(payload, color_b).as<uint8_t>();
            isRgbChannelCommand = true;
          }

          if (!isRgbChannelCommand) continue;

          /* set value */

          color_int = 0;

          color_int = color_r_value & 0xFF;
          (color_int) <<= 8;

          color_int |= color_g_value & 0xFF;
          (color_int) <<= 8;

          (color_int) |= color_b_value & 0xFF;
          supla_log(LOG_DEBUG,
                    "supla_client_set_rgbw(id: %d, color_int: %d, "
                    "color_brightness_value: %d, brightness_value: %d ",
                    id, color_int, color_brightness_value, brightness_value);

          supla_client_set_rgbw(supla_client, id, 0, color_int,
                                color_brightness_value, brightness_value, 1);

          /* --------- */

          return;
        } break;
      }
    } catch (jsoncons::json_exception& je) {
      supla_log(LOG_ERR,
                "error while trying get value from payload [error: %s]",
                je.what());
    } catch (std::exception& exception) {
      supla_log(LOG_ERR, "general error %s", exception.what());
    }
  }
}
