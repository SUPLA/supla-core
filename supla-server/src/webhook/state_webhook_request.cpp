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

#include "state_webhook_request.h"

#include <vector>

#include "device/channel_property_getter.h"
#include "http/asynctask_http_thread_pool.h"
#include "user/user.h"
#include "webhook/state_webhook_client.h"
#include "webhook/state_webhook_search_condition.h"
#include "webhook/state_webhook_throttling.h"

using std::shared_ptr;
using std::string;
using std::vector;

supla_state_webhook_request::supla_state_webhook_request(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    int actions, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter,
    supla_state_webhook_credentials *credentials)
    : supla_asynctask_http_request(caller, user_id, device_id, channel_id,
                                   queue, pool, property_getter) {
  this->actions = actions;
  this->credentials = credentials;
  this->timestamp = 0;
}

supla_state_webhook_request::~supla_state_webhook_request(void) {}

bool supla_state_webhook_request::is_any_action_set(void) { return actions; }

string supla_state_webhook_request::get_name(void) { return "Webhook"; }

void supla_state_webhook_request::set_timestamp(__time_t timestmap) {
  this->timestamp = timestmap;
}

bool supla_state_webhook_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!credentials->is_access_token_exists()) {
    return false;
  }

  supla_state_webhook_client client(get_channel_id(), curl_adapter, credentials,
                                    timestamp);

  if (actions) {
    return client.triggered_actions_report(actions);
  }

  int func = 0;
  supla_channel_availability_status status(true);
  supla_abstract_channel_value *value = get_channel_value(&func, &status);
  supla_channel_extended_value *extended_value = get_channel_extended_value();

  client.set_channel_connected(status.is_online());
  client.set_channel_value(value);
  client.set_channel_extended_value(extended_value);

  bool result = false;

  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
      result = client.power_switch_report();
      break;

    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      result = client.light_switch_report();
      break;

    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      result = client.staircase_timer_report();
      break;

    case SUPLA_CHANNELFNC_THERMOMETER:
      result = client.temperature_report();
      break;

    case SUPLA_CHANNELFNC_HUMIDITY:
      result = client.humidity_report();
      break;

    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      result = client.temperature_and_humidity_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      result = client.gateway_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      result = client.gate_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      result = client.garage_door_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      result = client.noliquid_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      result = client.door_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      result = client.roller_shutter_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
      result = client.roof_window_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      result = client.window_opening_sensor_report();
      break;

    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
      result = client.hotel_card_sensor_report();
      break;

    case SUPLA_CHANNELFNC_MAILSENSOR:
      result = client.mail_sensor_report();
      break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      result = client.shut_report("CONTROLLINGTHEROLLERSHUTTER");
      break;

    case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
      result = client.shut_report("CONTROLLINGTHEROOFWINDOW");
      break;
    case SUPLA_CHANNELFNC_TERRACE_AWNING:
      result = client.shut_report("TERRACE_AWNING");
      break;
    case SUPLA_CHANNELFNC_PROJECTOR_SCREEN:
      result = client.shut_report("PROJECTOR_SCREEN");
      break;
    case SUPLA_CHANNELFNC_CURTAIN:
      result = client.shut_report("CURTAIN");
      break;
    case SUPLA_CHANNELFNC_VERTICAL_BLIND:
      result = client.shut_report("VERTICAL_BLIND");
      break;
    case SUPLA_CHANNELFNC_ROLLER_GARAGE_DOOR:
      result = client.shut_report("ROLLER_GARAGE_DOOR");
      break;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      result = client.wind_sensor_report();
      break;

    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      result = client.pressure_sensor_report();
      break;

    case SUPLA_CHANNELFNC_RAINSENSOR:
      result = client.rain_sensor_report();
      break;

    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      result = client.weight_sensor_report();
      break;

    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      result = client.distance_sensor_report();
      break;

    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      result = client.depth_sensor_report();
      break;

    case SUPLA_CHANNELFNC_DIMMER:
      result = client.dimmer_report();
      break;

    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      result = client.dimmer_and_rgb_report();
      break;

    case SUPLA_CHANNELFNC_RGBLIGHTING:
      result = client.rgb_report();
      break;

    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
      result = client.electricity_measurement_report();
      break;

    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      result = client.impulse_counter_electricity_measurement_report();
      break;
    case SUPLA_CHANNELFNC_IC_GAS_METER:
      result = client.impulse_counter_gas_measurement_report();
      break;
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      result = client.impulse_counter_water_measurement_report();
      break;
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      result = client.impulse_counter_heat_measurement_report();
      break;
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
      result = client.hvac_report("HVAC_THERMOSTAT");
      break;
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
      result = client.hvac_report("HVAC_THERMOSTAT_HEAT_COOL");
      break;
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
      result = client.hvac_report("HVAC_THERMOSTAT_DIFFERENTIAL");
      break;
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
      result = client.hvac_report("HVAC_DOMESTIC_HOT_WATER");
      break;
  }

  return result;
}

// static
bool supla_state_webhook_request::is_caller_allowed(
    const supla_caller &caller) {
  switch (caller.get_type()) {
    case ctDevice:
    case ctChannel:
    case ctClient:
    case ctAmazonAlexa:
    case ctGoogleHome:
    case ctActionTrigger:
    case ctScene:
    case ctIPC:
    case ctCyclicTask:
    case ctMQTT:
      return true;
    default:
      return false;
  }

  return false;
}

// static
bool supla_state_webhook_request::is_function_allowed(
    int func, supla_state_webhook_credentials *credentials) {
  if (!credentials || !func) {
    return false;
  }

  vector<int> fids = credentials->get_function_ids();

  for (int f : fids) {
    if (f == func) {
      switch (func) {
        case SUPLA_CHANNELFNC_DIMMER:
        case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        case SUPLA_CHANNELFNC_RGBLIGHTING:
        case SUPLA_CHANNELFNC_THERMOMETER:
        case SUPLA_CHANNELFNC_HUMIDITY:
        case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
        case SUPLA_CHANNELFNC_WINDSENSOR:
        case SUPLA_CHANNELFNC_PRESSURESENSOR:
        case SUPLA_CHANNELFNC_RAINSENSOR:
        case SUPLA_CHANNELFNC_WEIGHTSENSOR:
        case SUPLA_CHANNELFNC_DISTANCESENSOR:
        case SUPLA_CHANNELFNC_DEPTHSENSOR:
        case SUPLA_CHANNELFNC_ELECTRICITY_METER:
        case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
        case SUPLA_CHANNELFNC_IC_GAS_METER:
        case SUPLA_CHANNELFNC_IC_WATER_METER:
        case SUPLA_CHANNELFNC_IC_HEAT_METER:
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
        case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_ROOFWINDOW:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
        case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
        case SUPLA_CHANNELFNC_MAILSENSOR:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROOFWINDOW:
        case SUPLA_CHANNELFNC_ACTIONTRIGGER:
        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
        case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
        case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
          return true;
      }
    }
  }

  return false;
}

// static
void supla_state_webhook_request::new_request(const supla_caller &caller,
                                              supla_user *user, int device_id,
                                              int channel_id, int actions) {
  if (!user || !is_caller_allowed(caller) || !user->stateWebhookCredentials() ||
      !user->stateWebhookCredentials()->is_access_token_exists() ||
      user->stateWebhookCredentials()->get_url().size() == 0) {
    return;
  }

  supla_channel_property_getter *property_getter =
      new supla_channel_property_getter();
  int func =
      property_getter->get_func(user->getUserID(), device_id, channel_id);

  if (!is_function_allowed(func, user->stateWebhookCredentials())) {
    delete property_getter;
    return;
  }

  // Refer to this "throttling" before looking for duplicates
  int delay_time_usec =
      supla_state_webhook_throttling::get_instance()->get_delay_time(channel_id,
                                                                     func);
  bool exists = false;
  supla_state_webhook_search_condition cnd(user->getUserID(), device_id,
                                           channel_id, actions);
  supla_asynctask_queue::global_instance()->access_task(
      &cnd, [&exists, actions](supla_abstract_asynctask *task) -> void {
        exists = true;
        if (actions) {
          supla_state_webhook_request *request =
              dynamic_cast<supla_state_webhook_request *>(task);
          if (request) {
            request->actions |= actions;
          }
        }
      });

  if (exists) {
    delete property_getter;
    return;
  }

  supla_state_webhook_request *request = new supla_state_webhook_request(
      caller, user->getUserID(), device_id, channel_id, actions,
      supla_asynctask_queue::global_instance(),
      supla_asynctask_http_thread_pool::global_instance(), property_getter,
      user->stateWebhookCredentials());
  request->set_delay_usec(delay_time_usec);
  if (actions) {
    request->set_priority(100);
  }
  request->start();
}
