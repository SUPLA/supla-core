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

#include "alexa_response_request.h"

#include "amazon/alexa_client.h"
#include "device/channel_property_getter.h"
#include "device/value/channel_hvac_value_with_temphum.h"
#include "http/asynctask_http_thread_pool.h"
#include "jsonconfig/channel/alexa_config.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;

supla_alexa_response_request::supla_alexa_response_request(
    const supla_caller &caller, int user_id, int device_id, int channel_id,
    supla_asynctask_queue *queue, supla_abstract_asynctask_thread_pool *pool,
    supla_abstract_channel_property_getter *property_getter,
    supla_amazon_alexa_credentials *credentials,
    supla_alexa_correlation_token *correlation_token)
    : supla_alexa_request(supla_caller(), user_id, device_id, channel_id, queue,
                          pool, property_getter, credentials) {
  set_delay_usec(
      5000000);  // 5 sec. - Try to send this request after ChangeReport.

  set_timeout(scfg_int(CFG_ALEXA_RESPONSE_TIMEOUT) * 1000);
  this->correlation_token = correlation_token;
}

supla_alexa_response_request::~supla_alexa_response_request(void) {
  if (correlation_token) {
    delete correlation_token;
  }
}

string supla_alexa_response_request::get_name(void) {
  return "Alexa Response Request";
}

bool supla_alexa_response_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!get_credentials()->is_access_token_exists() || !correlation_token ||
      correlation_token->get_token().empty()) {
    return false;
  }

  supla_alexa_client client(get_channel_id(), curl_adapter, get_credentials(),
                            get_zulu_time(), get_message_id(),
                            correlation_token->get_token());
  set_zulu_time("");
  set_message_id("");

  supla_channel_fragment fragment;
  bool online = false;

  supla_channel_value *value = get_channel_value(&fragment, &online);

  client.set_channel_connected(online);
  correlation_token->apply_expected(value);
  client.set_channel_value(value);

  client.set_subchannel_id(correlation_token->get_subchannel_id());
  client.set_cause_type(get_caller());

  switch (fragment.get_function()) {
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
      if (correlation_token->get_subchannel_id() == 1) {
        client.add_color_controller_properties();
      } else if (correlation_token->get_subchannel_id() == 2) {
        client.add_brightness_controller_properties();
      }
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
      client.add_percentage_controller();
      break;
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
      if (online) {
        supla_channel_hvac_value_with_temphum::expand(&value, &fragment,
                                                      get_property_getter());
        client.set_channel_value(value);
      }

      client.add_thermostat_controller();
      break;
  }

  return client.send_response();
}

// static
bool supla_alexa_response_request::is_caller_allowed(
    const supla_caller &caller) {
  return caller == ctAmazonAlexa;
}

// static
bool supla_alexa_response_request::is_function_allowed(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEFACADEBLIND:
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
void supla_alexa_response_request::new_request(
    const supla_caller &caller, supla_user *user, int device_id, int channel_id,
    supla_alexa_correlation_token *correlation_token) {
  if (user && user->amazonAlexaCredentials() &&
      user->amazonAlexaCredentials()->is_access_token_exists() &&
      correlation_token && is_caller_allowed(caller)) {
    supla_channel_property_getter *property_getter =
        new supla_channel_property_getter();

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

    if (is_function_allowed(func) && !integration_disabled) {
      supla_alexa_response_request *request = new supla_alexa_response_request(
          caller, user->getUserID(), device_id, channel_id,
          supla_asynctask_queue::global_instance(),
          supla_asynctask_http_thread_pool::global_instance(), property_getter,
          user->amazonAlexaCredentials(), correlation_token);

      request->set_priority(90);
      request->start();
      return;
    }

    delete property_getter;
  }

  if (correlation_token) {
    delete correlation_token;
  }
}
