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

#include "alexa_delete_request.h"

#include "amazon/alexa_client.h"
#include "amazon/alexa_delete_request_search_condition.h"
#include "amazon/alexa_discover_request.h"
#include "asynctask/voice_assistant_sync_thread_pool.h"
#include "db/db_access_provider.h"
#include "device/device_dao.h"
#include "jsonconfig/channel/alexa_config.h"
#include "svrcfg.h"
#include "user/user.h"

using std::string;
using std::vector;

supla_alexa_delete_request::supla_alexa_delete_request(
    int user_id, supla_asynctask_queue *queue,
    supla_abstract_asynctask_thread_pool *pool,
    supla_amazon_alexa_credentials *credentials)
    : supla_alexa_request(supla_caller(), user_id, 0, 0, queue, pool, nullptr,
                          credentials) {
  set_delay_usec(5000000);  // 5 sec.
  set_timeout(scfg_int(CFG_ALEXA_DELETE_REQUEST_TIMEOUT) * 1000);
}

string supla_alexa_delete_request::get_name(void) {
  return "Alexa Delete Request";
}

bool supla_alexa_delete_request::make_request(
    supla_abstract_curl_adapter *curl_adapter) {
  if (!get_credentials()->is_access_token_exists()) {
    return false;
  }

  lock();
  bool any_id = channel_ids.size() || scene_ids.size();
  unlock();

  if (!any_id) {
    return false;
  }

  supla_alexa_client client(get_channel_id(), curl_adapter, get_credentials(),
                            get_zulu_time(), get_message_id(), "");

  lock();
  for (auto it = channel_ids.cbegin(); it != channel_ids.cend(); ++it) {
    client.add_deleted_endpoint(*it, false);
  }

  for (auto it = scene_ids.cbegin(); it != scene_ids.cend(); ++it) {
    client.add_deleted_endpoint(*it, false);
  }
  unlock();

  return client.send_delete_report();
}

void supla_alexa_delete_request::add(std::vector<int> ids, bool scene) {
  lock();

  std::vector<int> *dst = scene ? &scene_ids : &channel_ids;

  for (auto it = ids.cbegin(); it != ids.cend(); ++it) {
    bool exists = false;
    for (auto dit = dst->cbegin(); dit != dst->cend(); ++dit) {
      if (*dit == *it) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      dst->push_back(*it);
    }
  }

  unlock();
}

// static
bool supla_alexa_delete_request::is_function_allowed(int func) {
  switch (func) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_HEAT_COOL:
    case SUPLA_CHANNELFNC_HVAC_THERMOSTAT_DIFFERENTIAL:
    case SUPLA_CHANNELFNC_HVAC_DOMESTIC_HOT_WATER:
    case SUPLA_CHANNELFNC_THERMOSTAT_HEATPOL_HOMEPLUS:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
    case SUPLA_CHANNELFNC_THERMOMETER:
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      return true;
    default:
      return false;
  }
}

// static
void supla_alexa_delete_request::new_request(supla_user *user, vector<int> ids,
                                             bool scene) {
  if (user || !ids.size()) {
    return;
  }

  bool exists = false;
  supla_alexa_delete_request_search_condition cnd(user->getUserID());
  supla_asynctask_queue::global_instance()->access_task(
      &cnd, [&exists, ids, scene](supla_abstract_asynctask *task) -> void {
        exists = true;
        dynamic_cast<supla_alexa_delete_request *>(task)->add(ids, scene);
      });

  if (!exists) {
    supla_alexa_delete_request *request = new supla_alexa_delete_request(
        user->getUserID(), supla_asynctask_queue::global_instance(),
        supla_voice_assistant_sync_thread_pool::global_instance(),
        user->amazonAlexaCredentials());
    request->add(ids, scene);
    request->start();
  }
}

// static
void supla_alexa_delete_request::new_request(supla_user *user, int channel_id,
                                             int func) {
  if (!user || !channel_id || !func || !is_function_allowed(func)) {
    return;
  }

  if (func) {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);

    supla_json_config *cfg =
        dao.get_channel_config(channel_id, nullptr, nullptr);
    if (cfg) {
      alexa_config acfg(cfg);
      if (!acfg.is_integration_disabled()) {
        channel_id = 0;
      }
      delete cfg;
    }
  }

  if (!channel_id) {
    return;
  };

  vector<int> ids;
  ids.push_back(channel_id);
  new_request(user, ids, false);
}

// static
void supla_alexa_delete_request::new_request(supla_user *user,
                                             int device_or_scene_id,
                                             bool scene) {
  if (!user || !device_or_scene_id) {
    return;
  }

  vector<int> ids;

  if (scene) {
    ids.push_back(device_or_scene_id);
  } else {
    supla_db_access_provider dba;
    supla_device_dao dao(&dba);

    vector<supla_channel_fragment> fragments =
        dao.get_channel_fragments(device_or_scene_id);

    for (auto it = fragments.begin(); it != fragments.end(); ++it) {
      if (is_function_allowed(it->get_function())) {
        ids.push_back(it->get_channel_id());
      }
    }
  }

  new_request(user, ids, scene);
}
