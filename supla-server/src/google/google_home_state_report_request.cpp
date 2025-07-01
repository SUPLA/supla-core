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

#include "google_home_state_report_request.h"

#include "device/channel_property_getter.h"
#include "device/value/channel_hvac_value_with_temphum.h"
#include "google/google_home_client.h"
#include "google/google_home_state_report_search_condition.h"
#include "google/google_home_state_report_throttling.h"
#include "http/asynctask_http_thread_pool.h"
#include "jsonconfig/channel/google_home_config.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_google_home_state_report_request::supla_google_home_state_report_request(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter,
    supla_google_home_credentials *credentials,
    supla_remote_gateway_access_token_provider *token_provider,
    const string &request_id)
    : supla_asynctask_http_request(supla_caller(), user_id, device_id,
                                   channel_id, queue, pool, property_getter) {
  this->credentials = credentials;
  this->token_provider = token_provider;
  this->request_id = request_id;

  set_timeout(scfg_int(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT) * 1000);
}

string supla_google_home_state_report_request::get_name(void) {
  return "Google Home State Report Request";
}

string supla_google_home_state_report_request::get_request_id(void) {
  lock();
  string result = request_id;
  unlock();

  return result;
}

void supla_google_home_state_report_request::set_request_id(
    const string &request_id) {
  lock();
  this->request_id = request_id;
  unlock();
}

bool supla_google_home_state_report_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!credentials->is_access_token_exists()) {
    return false;
  }

  supla_google_home_client client(
      get_channel_id(), curl_adapter, credentials,
      token_provider->get_token(platform_homegraph, 0));

  supla_channel_fragment fragment;
  supla_channel_availability_status status(true);

  supla_abstract_channel_value *value = get_channel_value(&fragment, &status);

  client.set_channel_connected(status.is_online());
  client.set_channel_value(value);
  client.set_request_id(get_request_id());
  set_request_id("");

  switch (fragment.get_function()) {
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
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
      client.add_facade_blind_state();
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      client.add_gate_state();
      break;
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      if (status.is_online()) {
        supla_channel_hvac_value_with_temphum::expand(&value, &fragment,
                                                      get_property_getter());
        client.set_channel_value(value);
      }

      client.add_thermostat_state();
      break;
    default:
      return false;
  }

  return client.state_report();
}

// static
bool supla_google_home_state_report_request::is_caller_allowed(
    const supla_caller &caller) {
  switch (caller.get_type()) {
    case ctDevice:
    case ctChannel:
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
bool supla_google_home_state_report_request::is_function_allowed(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      return true;
    default:
      return false;
  }

  return false;
}

// static
void supla_google_home_state_report_request::new_request(
    const supla_caller &caller, supla_user *user, int device_id, int channel_id,
    const std::string &request_id) {
  if (!user || !is_caller_allowed(caller) || !user->googleHomeCredentials() ||
      !user->googleHomeCredentials()->is_access_token_exists() ||
      user->googleHomeCredentials()->is_channel_excluded(channel_id)) {
    return;
  }

  supla_channel_property_getter *property_getter =
      new supla_channel_property_getter();

  int func =
      property_getter->get_func(user->getUserID(), device_id, channel_id);

  bool integration_disabled = false;
  {
    supla_json_config *config = property_getter->get_detached_json_config();
    if (config) {
      google_home_config gh_config(config);
      integration_disabled = gh_config.is_integration_disabled();
      delete config;
    }
  }

  if (!is_function_allowed(func) || integration_disabled) {
    delete property_getter;
    return;
  }

  // Refer to this "throttling" before looking for duplicates
  int delay_time_usec =
      supla_google_home_state_report_throttling::get_instance()->get_delay_time(
          channel_id, func);

  bool exists = false;
  supla_google_home_state_report_search_condition cnd(user->getUserID(),
                                                      device_id, channel_id);
  supla_asynctask_queue::global_instance()->access_task(
      &cnd, [&exists, request_id](supla_abstract_asynctask *task) -> void {
        exists = true;

        if (!request_id.empty()) {
          supla_google_home_state_report_request *request =
              dynamic_cast<supla_google_home_state_report_request *>(task);
          if (request) {
            request->set_request_id(request_id);
          }
        }
      });

  if (exists) {
    delete property_getter;
    return;
  }

  supla_google_home_state_report_request *request =
      new supla_google_home_state_report_request(
          caller, user->getUserID(), device_id, channel_id,
          supla_asynctask_queue::global_instance(),
          supla_asynctask_http_thread_pool::global_instance(), property_getter,
          user->googleHomeCredentials(),
          supla_remote_gateway_access_token_provider::global_instance(),
          request_id);

  request->set_priority(90);
  request->set_delay_usec(delay_time_usec);
  request->start();
}
