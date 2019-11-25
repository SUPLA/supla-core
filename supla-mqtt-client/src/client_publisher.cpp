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

#include "client_publisher.h"
#include <string>

void publish_mqtt_message_for_event(client_config* config,
                                    TSC_SuplaEvent* event) {
  if (event == NULL) return;
  if (config->getMqttPublishEvents() != true) return;

  std::string eventType = "UNKNOWN";

  switch (event->Event) {
    case SUPLA_EVENT_CONTROLLINGTHEGATEWAYLOCK: {
      eventType = "CONTROLLINGTHEGATEWAYLOCK";
    } break;
    case SUPLA_EVENT_CONTROLLINGTHEGATE: {
      eventType = "CONTROLLINGTHEGATE";
    } break;
    case SUPLA_EVENT_CONTROLLINGTHEGARAGEDOOR: {
      eventType = "CONTROLLINGTHEGARAGEDOOR";
    } break;
    case SUPLA_EVENT_CONTROLLINGTHEDOORLOCK: {
      eventType = "CONTROLLINGTHEDOORLOCK";
    } break;
    case SUPLA_EVENT_CONTROLLINGTHEROLLERSHUTTER: {
      eventType = "CONTROLLINGTHEROLLERSHUTTER";
    } break;
    case SUPLA_EVENT_POWERONOFF: {
      eventType = "POWERONOFF";
    } break;
    case SUPLA_EVENT_LIGHTONOFF: {
      eventType = "LIGHTONOFF";
    } break;
    case SUPLA_EVENT_STAIRCASETIMERONOFF: {
      eventType = "STAIRCASETIMERONOFF";
    } break;
  }

  std::string topic = "supla/channels/event";
  std::string payload =
      "{\"SenderName\": \"$SenderName$\", \"EventType\": \"$EventType$\" }";

  replace_string_in_place(&payload, "$SenderName$", event->SenderName);
  replace_string_in_place(&payload, "$EventType$", eventType);

  mqtt_client_publish(topic.c_str(), payload.c_str(), 0, 0);
}

void publish_mqtt_message_for_channel(client_device_channel* channel) {
  if (channel == NULL) return;

  bool online = channel->getOnline();
  for (auto state : channel->getStates()) {
    std::string caption(channel->getCaption());
    std::string payload = state->getPayload(channel->getId(), caption, online);
    std::string topic = state->getTopic(channel->getId(), channel->getType());

    double value;
    bool publish = false;
    switch (channel->getFunc()) {
      case SUPLA_CHANNELFNC_THERMOMETER:
      case SUPLA_CHANNELFNC_HUMIDITY:
      case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
        supla_channel_temphum* tempHum = channel->getTempHum();
        if (tempHum) {
          double temp = tempHum->getTemperature();
          double hum = tempHum->getHumidity();
          delete tempHum;

          replace_string_in_place(&payload, "$temperature$",
                                  std::to_string(temp));
          replace_string_in_place(&payload, "$humidity$", std::to_string(hum));

          publish = true;
        }
      } break;
      case SUPLA_CHANNELFNC_WINDSENSOR:
      case SUPLA_CHANNELFNC_PRESSURESENSOR:
      case SUPLA_CHANNELFNC_RAINSENSOR:
      case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      case SUPLA_CHANNELFNC_DEPTHSENSOR:
      case SUPLA_CHANNELFNC_DISTANCESENSOR: {
        channel->getDouble(&value);
        replace_string_in_place(&payload, "$value$", std::to_string(value));
        publish = true;
      } break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK: {
        char value[SUPLA_CHANNELVALUE_SIZE];
        char sub_value[SUPLA_CHANNELVALUE_SIZE];
        channel->getSubValue(sub_value);
        channel->getChar(value);
        bool hi = value[0] > 0;

        replace_string_in_place(&payload, "$value$", std::to_string(hi));
        replace_string_in_place(&payload, "$sensor_1$",
                                std::to_string(sub_value[0]));
        replace_string_in_place(&payload, "$sensor_2$",
                                std::to_string(sub_value[1]));
        publish = true;
      } break;
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      case SUPLA_CHANNELFNC_MAILSENSOR:
      case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER: {
        char cv[SUPLA_CHANNELVALUE_SIZE];
        channel->getChar(cv);
        bool hi = cv[0] > 0;

        replace_string_in_place(&payload, "$value$", std::to_string(hi));

        publish = true;
      } break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER: {
        char cv[SUPLA_CHANNELVALUE_SIZE];
        channel->getChar(cv);
        char sub_value[SUPLA_CHANNELVALUE_SIZE];
        channel->getSubValue(sub_value);
        char shut = cv[0];
        replace_string_in_place(&payload, "$shut$", std::to_string(shut));

        replace_string_in_place(&payload, "$sensor_1$",
                                std::to_string(sub_value[0]));
        replace_string_in_place(&payload, "$sensor_2$",
                                std::to_string(sub_value[1]));

        publish = true;
      } break;
      case SUPLA_CHANNELFNC_DIMMER:
      case SUPLA_CHANNELFNC_RGBLIGHTING:
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING: {
        int color;
        char color_brightness;
        char on_off;
        char brightness;
        channel->getRGBW(&color, &color_brightness, &brightness, &on_off);

        unsigned char blue = (unsigned char)((color & 0x000000FF));
        unsigned char green = (unsigned char)((color & 0x0000FF00) >> 8);
        unsigned char red = (unsigned char)((color & 0x00FF0000) >> 16);

        char hex_color[11];
        snprintf(hex_color, sizeof(hex_color), "0x%02X%02X%02X", red, green,
                 blue);

        replace_string_in_place(&payload, "$red$", std::to_string(red));
        replace_string_in_place(&payload, "$green$", std::to_string(green));
        replace_string_in_place(&payload, "$blue$", std::to_string(blue));

        replace_string_in_place(&payload, "$color$", hex_color);
        replace_string_in_place(&payload, "$color_brightness$",
                                std::to_string(color_brightness));
        replace_string_in_place(&payload, "$brightness$",
                                std::to_string(brightness));
        replace_string_in_place(&payload, "$value$", std::to_string(on_off));
        publish = true;
      } break;
      case SUPLA_CHANNELFNC_ELECTRICITY_METER: {
        TSuplaChannelExtendedValue* value = (TSuplaChannelExtendedValue*)malloc(
            sizeof(TSuplaChannelExtendedValue));

        if (!channel->getExtendedValue(value)) {
          free(value);
          break;
        }

        TElectricityMeter_ExtendedValue em_ev;
        TSC_ImpulseCounter_ExtendedValue ic_ev;

        if (srpc_evtool_v1_extended2icextended(value, &ic_ev)) {
          std::string currency(ic_ev.currency, 3);
          std::string custom_unit(ic_ev.custom_unit, 9);

          replace_string_in_place(&payload, "$currency$", currency);
          replace_string_in_place(
              &payload, "$pricePerUnit$",
              std::to_string(ic_ev.price_per_unit * 0.0001));
          replace_string_in_place(&payload, "$totalCost$",
                                  std::to_string(ic_ev.total_cost * 0.01));
          replace_string_in_place(&payload, "$impulsesPerUnit$",
                                  std::to_string(ic_ev.impulses_per_unit));
          replace_string_in_place(&payload, "$counter$",
                                  std::to_string(ic_ev.counter));
          replace_string_in_place(&payload, "$calculatedValue$",
                                  std::to_string(ic_ev.calculated_value));
          replace_string_in_place(&payload, "$unit$", custom_unit);

          publish = true;

        } else if (srpc_evtool_v1_extended2emextended(value, &em_ev) == 1) {
          std::string currency(em_ev.currency, 3);

          replace_string_in_place(&payload, "$currency$", currency);
          replace_string_in_place(
              &payload, "$pricePerUnit$",
              std::to_string(em_ev.price_per_unit * 0.0001));
          replace_string_in_place(&payload, "$totalCost$",
                                  std::to_string(em_ev.total_cost * 0.01));

          for (int a = 0; a < 3; a++) {
            if (em_ev.m_count > 0 && em_ev.m[0].voltage[a] > 0) {
              std::string stra = std::to_string(a) + "$";
              replace_string_in_place(&payload, "$number_" + stra,
                                      std::to_string(a + 1));
              replace_string_in_place(&payload, "$frequency_" + stra,
                                      std::to_string(em_ev.m[0].freq * 0.01));
              replace_string_in_place(
                  &payload, "$voltage_" + stra,
                  std::to_string(em_ev.m[0].voltage[a] * 0.01));
              replace_string_in_place(
                  &payload, "$current_" + stra,
                  std::to_string(em_ev.m[0].current[a] * 0.001));
              replace_string_in_place(
                  &payload, "$powerActive_" + stra,
                  std::to_string(em_ev.m[0].power_active[a] * 0.00001));
              replace_string_in_place(
                  &payload, "$powerReactive_" + stra,
                  std::to_string(em_ev.m[0].power_reactive[a] * 0.00001));
              replace_string_in_place(
                  &payload, "$powerApparent_" + stra,
                  std::to_string(em_ev.m[0].power_apparent[a] * 0.00001));
              replace_string_in_place(
                  &payload, "$powerFactor_" + stra,
                  std::to_string(em_ev.m[0].power_factor[a] * 0.001));
              replace_string_in_place(
                  &payload, "$phaseAngle_" + stra,
                  std::to_string(em_ev.m[0].phase_angle[a] * 0.1));
              replace_string_in_place(
                  &payload, "$totalForwardActiveEnergy_" + stra,
                  std::to_string(em_ev.total_forward_active_energy[a] *
                                 0.00001));
              replace_string_in_place(
                  &payload, "$totalReverseActiveEnergy_" + stra,
                  std::to_string(em_ev.total_reverse_active_energy[a] *
                                 0.00001));
              replace_string_in_place(
                  &payload, "$totalForwardReactiveEnergy_" + stra,
                  std::to_string(em_ev.total_forward_reactive_energy[a] *
                                 0.0001));
              replace_string_in_place(
                  &payload, "$totalReverseReactiveEnergy_" + stra,
                  std::to_string(em_ev.total_reverse_reactive_energy[a] *
                                 0.0001));
            }
          }

          publish = true;
        }
        free(value);
      } break;
    }
    if (publish) {
      mqtt_client_publish(topic.c_str(), payload.c_str(), 1, 0);
    }
  }
}
