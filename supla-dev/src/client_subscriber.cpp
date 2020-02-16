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

void handle_subscribed_message(client_device_channel* channel,
                               std::string topic, std::string message,
                               _func_channelio_valuechanged cb,
                               void* user_data) {
  if (message.length() == 0) return;

  while (!cb) {
    supla_log(LOG_DEBUG, "waiting for registration result...");
    usleep(10);
  }

  supla_log(LOG_DEBUG, "handling message %s", message.c_str());

  char value[SUPLA_CHANNELVALUE_SIZE];

  channel->getValue(value);
  int channelNumber = channel->getNumber();

  jsoncons::json payload;
  std::string template_value = message;

  try {
    payload = jsoncons::json::parse(message);

    if (channel->getPayloadValue().length() > 0) {
      std::string payloadValue = std::string(channel->getPayloadValue());
      template_value =
          jsoncons::jsonpointer::get(payload, payloadValue).as<std::string>();
      if (template_value.length() == 0) return;
    }
  } catch (jsoncons::ser_error& ser) {
  } catch (jsoncons::jsonpointer::jsonpointer_error& error) {
  } catch (jsoncons::parse_error& per) {
  } catch (jsoncons::parse_exception& pex) {
  }

  supla_log(LOG_DEBUG, "handling incomming message: %s",
            template_value.c_str());
  try {
    /* raw payload simple value */
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE: {
        if (channel->getPayloadOn().compare(template_value) == 0)
          value[0] = 1;
        else
          value[0] = 0;

        channel->setValue(value);
        cb(channelNumber, value, user_data);

        return;
      } break;
      case SUPLA_CHANNELFNC_DISTANCESENSOR:
      case SUPLA_CHANNELFNC_DEPTHSENSOR:
      case SUPLA_CHANNELFNC_WINDSENSOR:
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
      case SUPLA_CHANNELFNC_RAINSENSOR:
      case SUPLA_CHANNELFNC_WEIGHTSENSOR: {
        double dbval = std::stod(template_value);
        memcpy(value, &dbval, sizeof(double));
        channel->setValue(value);
        cb(channel->getNumber(), value, user_data);
        return;
      };
      case SUPLA_CHANNELFNC_THERMOMETER: {
        std::string::size_type sz;  // alias of size_t
        double temp = std::stod(template_value, &sz);
        supla_channel_temphum* temphum = channel->getTempHum();

        if (temphum) {
          temphum->setTemperature(temp);
          temphum->toValue(value);
        }

        channel->setValue(value);
        cb(channelNumber, value, user_data);

        return;

      } break;
      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
        std::string::size_type sz;  // alias of size_t
        double temp = std::stod(template_value, &sz);
        double hum = std::stod(template_value.substr(sz));

        supla_channel_temphum* temphum = channel->getTempHum();

        if (temphum) {
          temphum->setTemperature(temp);
          temphum->setHumidity(hum);
          temphum->toValue(value);
        }

        channel->setValue(value);
        cb(channelNumber, value, user_data);

        return;

      } break;
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:  // ver. >= 8
      case SUPLA_CHANNELFNC_MAILSENSOR:            // ver. >= 8
      {
        value[0] = channel->getPayloadOn().compare(template_value) == 0 ? 1 : 0;
        ;
        channel->setValue(value);
        cb(channel->getNumber(), value, user_data);
        return;
      }
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER: {
        auto temp = atoi(template_value.c_str());
        if (temp < 0) temp = 0;
        if (temp > 100) temp = 100;

        value[0] = temp;

        channel->setValue(value);
        cb(channel->getNumber(), value, user_data);
        return;
      } break;
    };

    /*

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

          supla_log(LOG_DEBUG, "handling %d %s %s", channel->getFunc(),
                    value.c_str(), on_value.c_str());

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



          return;
        } break;
      }*/
  } catch (jsoncons::json_exception& je) {
    supla_log(LOG_ERR, "error while trying get value from payload [error: %s]",
              je.what());
  } catch (std::exception& exception) {
    supla_log(LOG_ERR, "general error %s", exception.what());
  }
}
