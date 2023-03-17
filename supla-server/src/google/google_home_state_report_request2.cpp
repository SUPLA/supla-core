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

#include "google/google_home_state_report_request2.h"

#include "google/google_home_client2.h"
#include "http/asynctask_http_thread_pool.h"
#include "svrcfg.h"

using std::string;

supla_google_home_state_report_request2::
    supla_google_home_state_report_request2(
        const supla_caller &caller, int user_id, int device_id, int channel_id,
        supla_asynctask_queue *queue,
        supla_abstract_asynctask_thread_pool *pool,
        supla_abstract_channel_property_getter *property_getter,
        supla_google_home_credentials2 *credentials, const string &request_id)
    : supla_asynctask_http_request(supla_caller(), user_id, device_id,
                                   channel_id, queue, pool, property_getter) {
  this->credentials = credentials;
  this->request_id = request_id;

  set_delay_usec(1500000);  // 1.5 sec.
  set_timeout(scfg_int(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT) * 1000);
}

string supla_google_home_state_report_request2::get_name(void) {
  return "Google Home State Report Request";
}

bool supla_google_home_state_report_request2::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!credentials->is_access_token_exists()) {
    return false;
  }

  supla_google_home_client2 client(get_channel_id(), curl_adapter, credentials);

  int func = 0;
  bool online = false;

  supla_channel_value *value = get_channel_value(&func, &online);

  client.set_channel_connected(online);
  client.set_channel_value(value);
  client.set_request_id(request_id);

  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      client.add_onoff_state();
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      client.add_brightness_state();
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      client.add_color_state();
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      client.set_subchannel_id(1);
      client.add_color_state();
      client.set_subchannel_id(2);
      client.add_brightness_state();
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      client.add_roller_shutter_state();
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      client.add_gate_state();
      break;
    default:
      delete value;
      return false;
  }

  bool result = client.state_report();
  delete value;

  return result;
}

// static
bool supla_google_home_state_report_request2::is_caller_allowed(
    const supla_caller &caller) {
  switch (caller.get_type()) {
    case ctDevice:
    case ctClient:
    case ctIPC:
    case ctMQTT:
    case ctGoogleHome:
    case ctAmazonAlexa:
    case ctScene:
    case ctActionTrigger:
    case ctCyclicTask:
      return true;
    case ctUnknown:
      return false;
  }

  return false;
}

// static
bool supla_google_home_state_report_request2::is_function_allowed(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return true;
    default:
      return false;
  }

  return false;
}

// static
void supla_google_home_state_report_request2::new_request(
    const supla_caller &caller, supla_user *user, int device_id, int channel_id,
    const std::string &request_id) {
  // reject disabled channels (json googleHomeDisabled == true)
}
