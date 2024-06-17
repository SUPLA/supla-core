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

#include "alexa_discover_payload_obtainer.h"

#include <string.h>

#include "json/cJSON.h"
#include "log.h"
#include "tools.h"

using std::string;

supla_alexa_discover_payload_obtainer::supla_alexa_discover_payload_obtainer(
    supla_amazon_alexa_credentials_dao *dao) {
  this->dao = dao;
}

supla_alexa_discover_payload_obtainer::~supla_alexa_discover_payload_obtainer(
    void) {
  if (dao) {
    delete dao;
  }
}

string supla_alexa_discover_payload_obtainer::get_message_id(void) {
  char msgid[37] = {};
  st_uuid_v4(msgid);

  return msgid;
}

cJSON *supla_alexa_discover_payload_obtainer::json_get_object(
    cJSON *root, const char name[]) {
  cJSON *obj = cJSON_GetObjectItem(root, name);
  if (!obj) {
    obj = cJSON_AddObjectToObject(root, name);
  }

  return obj;
}

void supla_alexa_discover_payload_obtainer::json_set_string(
    cJSON *root, const char name[], const char value[]) {
  cJSON *item = cJSON_GetObjectItem(root, name);
  if (item) {
    if (cJSON_IsString(item)) {
      cJSON_SetValuestring(item, value);
    }
  } else {
    cJSON_AddStringToObject(root, name, value);
  }
}

string supla_alexa_discover_payload_obtainer::obtain(
    int user_id, supla_amazon_alexa_credentials *credentials,
    supla_abstract_curl_adapter *curl_adapter) {
  if (!curl_adapter || !credentials || !dao) {
    return "";
  }

  string token = dao->get_cloud_access_token(user_id);
  if (token.empty()) {
    supla_log(LOG_ERR, "Failed to obtain cloud token.");
    return "";
  }

  cJSON *json_root = cJSON_CreateObject();
  cJSON *directive = cJSON_AddObjectToObject(json_root, "directive");
  cJSON *header = cJSON_AddObjectToObject(directive, "header");
  cJSON_AddStringToObject(header, "namespace", "Alexa.Discovery");
  cJSON_AddStringToObject(header, "name", "Discover");
  cJSON_AddStringToObject(header, "payloadVersion", "3");
  cJSON_AddStringToObject(header, "messageId", get_message_id().c_str());

  cJSON *payload = cJSON_AddObjectToObject(directive, "payload");
  cJSON *scope = cJSON_AddObjectToObject(payload, "scope");
  cJSON_AddStringToObject(scope, "type", "BearerToken");
  cJSON_AddStringToObject(scope, "token", token.c_str());

  char *data = cJSON_PrintUnformatted(json_root);
  cJSON_Delete(json_root);

  if (!data) {
    return "";
  }

  string request_result;

  curl_adapter->reset();
  curl_adapter->set_opt_write_data(&request_result);
  curl_adapter->append_header("Content-Type: application/json");
  curl_adapter->set_opt_post_fields(data);
  curl_adapter->set_opt_url(
      "https://kune6om4mlleevu2kh4vt2ic5i0otrmw.lambda-url.eu-west-1.on.aws");

  long response_code = 0;

  if (curl_adapter->perform() &&
      (response_code = curl_adapter->get_response_code()) == 200) {
    free(data);
    data = nullptr;

    cJSON *response = cJSON_Parse(request_result.c_str());
    request_result.clear();

    if (response) {
      cJSON *event = cJSON_GetObjectItem(response, "event");
      if (event) {
        payload = json_get_object(event, "payload");
        if (payload) {
          scope = json_get_object(payload, "scope");
          if (scope) {
            json_set_string(scope, "type", "BearerToken");
            json_set_string(scope, "token",
                            credentials->get_access_token().c_str());
          }
        }

        header = cJSON_GetObjectItem(event, "header");
        if (header) {
          cJSON *name = cJSON_GetObjectItem(header, "name");
          cJSON *messageId = cJSON_GetObjectItem(header, "messageId");

          if (name && cJSON_IsString(name) && messageId &&
              cJSON_IsString(messageId) &&
              strncmp(cJSON_GetStringValue(name), "Discover.Response", 20) ==
                  0) {
            cJSON_SetValuestring(name, "AddOrUpdateReport");
            cJSON_SetValuestring(messageId, get_message_id().c_str());
            data = cJSON_PrintUnformatted(response);

            if (data) {
              request_result = data;
              free(data);
              data = nullptr;
            }
          }
        }
      }
      cJSON_Delete(response);

      if (request_result.empty()) {
        supla_log(LOG_ERR,
                  "The received payload for Alexa Discovery is empty or in "
                  "the wrong format.");
      }
    }

  } else {
    free(data);
    data = nullptr;
    request_result.clear();

    supla_log(LOG_ERR,
              "Unsuccessful attempt to obtain a payload for Alexa Discovery. "
              "Response code: %i",
              response_code);
  }

  return request_result;
}
