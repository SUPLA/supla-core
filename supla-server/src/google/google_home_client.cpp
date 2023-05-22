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

#include "google_home_client.h"

#include <string.h>

#include "device/value/channel_gate_value.h"
#include "device/value/channel_onoff_value.h"
#include "device/value/channel_rgbw_value.h"
#include "device/value/channel_rs_value.h"
#include "log.h"
#include "tools.h"

using std::string;

supla_google_home_client::supla_google_home_client(
    int channel_id, supla_abstract_curl_adapter *curl_adapter,
    supla_google_home_credentials *credentials)
    : supla_voice_assistant_client(channel_id, curl_adapter, credentials) {
  this->json_states = cJSON_CreateObject();
}

supla_google_home_client::~supla_google_home_client(void) {
  cJSON_Delete(json_states);
}

supla_google_home_credentials *supla_google_home_client::get_gh_credentials(
    void) {
  return dynamic_cast<supla_google_home_credentials *>(get_credentials());
}

void supla_google_home_client::set_request_id(const string &request_id) {
  this->request_id = request_id;
}

bool supla_google_home_client::perform_post_request(cJSON *json_data,
                                                    int *http_result_code) {
  bool result = false;

  if (!get_credentials()->is_access_token_exists()) {
    return false;
  }

  char *data = cJSON_PrintUnformatted(json_data);
  cJSON_Delete((cJSON *)json_data);

  if (data) {
    string request_result;
    string token = "Authorization: Bearer ";
    token.append(get_credentials()->get_access_token());

    get_curl_adapter()->reset();
    get_curl_adapter()->set_opt_url(
        "https://"
        "odokilkqoesh73zfznmiupey4a0uugaz.lambda-url.eu-west-1.on.aws/");
    get_curl_adapter()->append_header("Content-Type: application/json");
    get_curl_adapter()->append_header(token.c_str());
    get_curl_adapter()->set_opt_post_fields(data);

    get_curl_adapter()->set_opt_write_data(&request_result);

    if (get_curl_adapter()->perform()) {
      if (http_result_code) {
        *http_result_code = get_curl_adapter()->get_response_code();
      }
      result = get_curl_adapter()->get_response_code() == 200;

      if (!result) {
        supla_log(
            LOG_ERR,
            "GoogleHomeGraph client error userId: %i, code=%i, message=%s",
            get_credentials()->get_user_id(),
            get_curl_adapter()->get_response_code(), request_result.c_str());
      }
    }

    free(data);
  }

  return result;
}

cJSON *supla_google_home_client::get_header(void) {
  cJSON *header = cJSON_CreateObject();
  if (header) {
    const char name[] = "requestId";

    if (request_id.size() == 0) {
      char request_id[37] = {};
      st_uuid_v4(request_id);
      this->request_id = request_id;
    }

    cJSON_AddStringToObject(header, name, request_id.c_str());
    cJSON_AddStringToObject(
        header, "agentUserId",
        get_credentials()->get_user_long_unique_id().c_str());

    return header;
  }

  return NULL;
}

bool supla_google_home_client::channel_exists(const char *endpoint_id) {
  return endpoint_id != NULL &&
         cJSON_GetObjectItem(json_states, endpoint_id) != NULL;
}

cJSON *supla_google_home_client::get_state_skeleton(void) {
  cJSON *state = NULL;
  string endpoint_id = get_endpoint_id();
  if (endpoint_id.size()) {
    if (!channel_exists(endpoint_id.c_str())) {
      state = cJSON_CreateObject();
      if (state) {
        cJSON_AddBoolToObject(state, "online", is_channel_connected());
        cJSON_AddItemToObject(json_states, endpoint_id.c_str(), state);
      }
    }
  }

  return state;
}

void supla_google_home_client::add_onoff_state(void) {
  cJSON *state = (cJSON *)get_state_skeleton();
  if (state) {
    supla_channel_onoff_value *v =
        dynamic_cast<supla_channel_onoff_value *>(get_channel_value());

    cJSON_AddBoolToObject(state, "on",
                          is_channel_connected() && v && v->is_on());
  }
}

void supla_google_home_client::add_brightness_state(void) {
  cJSON *state = (cJSON *)get_state_skeleton();
  if (state) {
    supla_channel_rgbw_value *v =
        dynamic_cast<supla_channel_rgbw_value *>(get_channel_value());

    cJSON_AddBoolToObject(
        state, "on", is_channel_connected() && v && v->get_brightness() > 0);
    cJSON_AddNumberToObject(
        state, "brightness",
        is_channel_connected() && v ? v->get_brightness() : 0);
  }
}

void supla_google_home_client::add_color_state(void) {
  cJSON *state = (cJSON *)get_state_skeleton();
  if (state) {
    cJSON *json_color = cJSON_CreateObject();
    if (json_color) {
      supla_channel_rgbw_value *v =
          dynamic_cast<supla_channel_rgbw_value *>(get_channel_value());

      cJSON_AddNumberToObject(json_color, "spectrumRGB",
                              v ? v->get_color() : 0);
      cJSON_AddItemToObject(state, "color", json_color);

      cJSON_AddBoolToObject(
          state, "on",
          is_channel_connected() && v && v->get_color_brightness() > 0);
      cJSON_AddNumberToObject(
          state, "brightness",
          is_channel_connected() && v ? v->get_color_brightness() : 0);
    }
  }
}

void supla_google_home_client::add_open_percent_state(short open_percent) {
  cJSON *state = (cJSON *)get_state_skeleton();
  if (state) {
    cJSON_AddNumberToObject(state, "openPercent", open_percent);
  }
}

void supla_google_home_client::add_gate_state(void) {
  supla_channel_gate_value *v =
      dynamic_cast<supla_channel_gate_value *>(get_channel_value());

  add_open_percent_state(is_channel_connected() && v &&
                                 v->get_opening_sensor_level() == gsl_open
                             ? 100
                             : 0);
}

void supla_google_home_client::add_roller_shutter_state(void) {
  supla_channel_rs_value *v =
      dynamic_cast<supla_channel_rs_value *>(get_channel_value());

  short shut_percentage = is_channel_connected() && v &&
                                  v->get_rs_value()->position >= 0 &&
                                  v->get_rs_value()->position <= 100
                              ? v->get_rs_value()->position
                              : 0;

  add_open_percent_state(100 - shut_percentage);
}

bool supla_google_home_client::state_report(void) {
  cJSON *report = (cJSON *)get_header();

  if (report) {
    cJSON *payload = cJSON_CreateObject();
    if (payload) {
      cJSON *devices = cJSON_CreateObject();
      if (devices) {
        cJSON_AddItemToObject(devices, "states", json_states);
        cJSON_AddItemToObject(payload, "devices", devices);
        cJSON_AddItemToObject(report, "payload", payload);

        json_states = cJSON_CreateObject();

        int http_result_code = 0;
        if (perform_post_request(report, &http_result_code)) {
          return http_result_code >= 200 && http_result_code <= 206;
        } else if (http_result_code == 403 || http_result_code == 404) {
          // Previously, we forced re-sync, but as it turns out, nothing
          // changes, and in addition, SYNC sometimes returns 404 even though
          // the synchronization is going through correctly.
          get_gh_credentials()->exclude_channel(get_channel_id());
        }
      }
    }
  }

  return false;
}

bool supla_google_home_client::sync(void) {
  cJSON *header = (cJSON *)get_header();

  if (header) {
    cJSON_AddStringToObject(header, "intent", "action.devices.SYNC");
    int http_result_code = 0;
    if (perform_post_request(header, &http_result_code)) {
      return http_result_code >= 200 && http_result_code <= 206;
    }
  }

  return false;
}
