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

#include "alexa_change_report_request.h"

#include "amazon/alexa_change_report_search_condition.h"
#include "amazon/alexa_change_report_throttling.h"
#include "amazon/alexa_client.h"
#include "device/channel_property_getter.h"
#include "http/asynctask_http_thread_pool.h"
#include "jsonconfig/channel/alexa_config.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_alexa_change_report_request::supla_alexa_change_report_request(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter,
    supla_amazon_alexa_credentials *credentials)
    : supla_alexa_request(caller, user_id, device_id, channel_id, queue, pool,
                          property_getter, credentials) {
  set_timeout(scfg_int(CFG_ALEXA_CHANGEREPORT_TIMEOUT) * 1000);
}

string supla_alexa_change_report_request::get_name(void) {
  return "Alexa Change Report Request";
}

bool supla_alexa_change_report_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!get_credentials()->is_access_token_exists()) {
    return false;
  }

  supla_alexa_client client(get_channel_id(), curl_adapter, get_credentials(),
                            get_zulu_time(), get_message_id(), "");
  set_zulu_time("");
  set_message_id("");

  int func = 0;
  bool online = false;

  supla_channel_value *value = get_channel_value(&func, &online);

  client.set_channel_connected(online);
  client.set_channel_value(value);
  client.set_cause_type(get_caller());

  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
      client.add_power_controller_properties();
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      client.add_brightness_controller_properties();
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      client.add_color_controller_properties();
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      client.set_subchannel_id(1);
      client.add_color_controller_properties();

      if (client.send_change_report()) {
        client.set_subchannel_id(2);
        client.add_brightness_controller_properties();
        return client.send_change_report();
      }

      return false;

    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
      client.add_contact_sensor();
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      client.add_range_controller();
      client.add_percentage_controller();
      break;
  }

  return client.send_change_report();
}

// static
bool supla_alexa_change_report_request::is_caller_allowed(
    const supla_caller &caller) {
  switch (caller.get_type()) {
    case ctDevice:
    case ctClient:
    case ctAmazonAlexa:
    case ctIPC:
    case ctMQTT:
    case ctScene:
    case ctCyclicTask:
    case ctActionTrigger:
      return true;
    case ctUnknown:
    case ctGoogleHome:
      return false;
  }

  return false;
}

// static
bool supla_alexa_change_report_request::is_function_allowed(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_HOTELCARDSENSOR:
      return true;
    default:
      return false;
  }

  return false;
}

// static
void supla_alexa_change_report_request::new_request(const supla_caller &caller,
                                                    supla_user *user,
                                                    int device_id,
                                                    int channel_id) {
  if (!user || !is_caller_allowed(caller) || !user->amazonAlexaCredentials() ||
      !user->amazonAlexaCredentials()->is_access_token_exists()) {
    return;
  }

  supla_cahnnel_property_getter *property_getter =
      new supla_cahnnel_property_getter();

  int func =
      property_getter->get_func(user->getUserID(), device_id, channel_id);

  bool integration_disabled = false;
  {
    supla_json_config *config = property_getter->get_detached_json_config();
    if (config) {
      alexa_config a_config(config);
      integration_disabled = a_config.is_integration_disabled();
      delete config;
    }
  }

  if (!is_function_allowed(func) || integration_disabled) {
    delete property_getter;
    return;
  }

  // Refer to this "throttling" before looking for duplicates
  int delay_time_usec =
      supla_alexa_change_report_throttling::get_instance()->get_delay_time(
          channel_id, func);

  bool exists = false;
  supla_alexa_change_report_search_condition cnd(user->getUserID(), device_id,
                                                 channel_id);
  supla_asynctask_queue::global_instance()->access_task(
      &cnd,
      [&exists](supla_abstract_asynctask *task) -> void { exists = true; });

  if (exists) {
    delete property_getter;
    return;
  }

  supla_alexa_change_report_request *request =
      new supla_alexa_change_report_request(
          caller, user->getUserID(), device_id, channel_id,
          supla_asynctask_queue::global_instance(),
          supla_asynctask_http_thread_pool::global_instance(), property_getter,
          user->amazonAlexaCredentials());

  request->set_priority(90);
  request->set_delay_usec(delay_time_usec);
  request->start();
}
