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

#include "abstract_get_channel_state_command.h"

#include <arpa/inet.h>
#include <string.h>

#include "ipc/abstract_ipc_ctrl.h"
#include "tools.h"

using std::string;

supla_abstract_get_channel_state_command::
    supla_abstract_get_channel_state_command(
        supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_command(socket_adapter) {}

const string supla_abstract_get_channel_state_command::get_command_name(void) {
  return "GET-CHANNEL-STATE:";
}

void supla_abstract_get_channel_state_command::on_command_match(
    const char *params) {
  int user_id = 0;
  int device_id = 0;
  int channel_id = 0;
  int get_from_extended = 0;
  int send_request = 0;

  if (params) {
    sscanf(params, "%i,%i,%i,%i,%i", &user_id, &device_id, &channel_id,
           &send_request, &get_from_extended);

    if (user_id && device_id && channel_id) {
      TDSC_ChannelState state = {};
      if (get_channel_state(&state, user_id, device_id, channel_id,
                            send_request > 0, get_from_extended > 0)) {
        string result = "STATE:" + std::to_string(channel_id) + ",";

        if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT)) {
          result.append(std::to_string(state.defaultIconField));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_SWITCHCYCLECOUNT) {
          result.append(std::to_string(state.SwitchCycleCount));
        }

        result.append(",");

        char ipv4[INET_ADDRSTRLEN] = {};

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_IPV4) {
          struct in_addr addr;
          addr.s_addr = htonl(state.IPv4);

          if (inet_ntop(AF_INET, &addr, ipv4, INET_ADDRSTRLEN) == nullptr) {
            ipv4[0] = 0;
          }
        }

        result.append(ipv4);
        result.append(",");

        char mac[18] = {};

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_MAC) {
          snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                   (unsigned char)state.MAC[0], (unsigned char)state.MAC[1],
                   (unsigned char)state.MAC[2], (unsigned char)state.MAC[3],
                   (unsigned char)state.MAC[4], (unsigned char)state.MAC[5]);
        }

        result.append(mac);
        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL) {
          result.append(std::to_string(state.BatteryLevel));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED) {
          result.append(std::to_string(state.BatteryPowered ? 1 : 0));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_WIFIRSSI) {
          result.append(std::to_string(state.WiFiRSSI));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_WIFISIGNALSTRENGTH) {
          result.append(std::to_string(state.WiFiSignalStrength));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BRIDGENODEONLINE) {
          result.append(std::to_string(state.BridgeNodeOnline ? 1 : 0));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BRIDGENODESIGNALSTRENGTH) {
          result.append(std::to_string(state.BridgeNodeSignalStrength));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_UPTIME) {
          result.append(std::to_string(state.Uptime));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_CONNECTIONUPTIME) {
          result.append(std::to_string(state.ConnectionUptime));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_BATTERYHEALTH) {
          result.append(std::to_string(state.BatteryHealth));
        }

        result.append(",");

        if (state.Fields & SUPLA_CHANNELSTATE_FIELD_LASTCONNECTIONRESETCAUSE) {
          result.append(std::to_string(state.LastConnectionResetCause));
        }

        result.append(",");

        if ((state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN) &&
            !(state.Fields &
              SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
            !(state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
          result.append(std::to_string(state.LightSourceLifespan));
          result.append(",");
          result.append(std::to_string(state.LightSourceLifespanLeft));
        } else {
          result.append(",");
        }

        result.append(",");

        if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN) &&
            (state.Fields &
             SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
            !(state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
          result.append(std::to_string(state.LightSourceOperatingTime));
        }

        result.append(",");

        if (!(state.Fields & SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCELIFESPAN) &&
            !(state.Fields &
              SUPLA_CHANNELSTATE_FIELD_LIGHTSOURCEOPERATINGTIME) &&
            (state.Fields & SUPLA_CHANNELSTATE_FIELD_OPERATINGTIME)) {
          result.append(std::to_string(state.OperatingTime));
        }

        send_result(result.c_str());

      } else {
        send_result("NOT-AVAILABLE:", channel_id);
      }

      return;
    }
  }

  send_result("UNKNOWN:", user_id);
}
