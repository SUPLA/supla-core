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

string supla_alexa_discover_payload_obtainer::obtain(
    int user_id, supla_abstract_curl_adapter *curl_adapter) {
  if (!curl_adapter || !dao) {
    return "";
  }

  string token = dao->get_cloud_access_token(user_id);
  if (token.empty()) {
    supla_log(LOG_ERR, "Failed to obtain cloud token.");
    return "";
  }

  char msgid[37] = {};
  st_uuid_v4(msgid);

  cJSON *json_root = cJSON_CreateObject();
  cJSON *directive = cJSON_AddObjectToObject(json_root, "directive");
  cJSON *header = cJSON_AddObjectToObject(directive, "header");
  cJSON_AddStringToObject(header, "namespace", "Alexa.Discovery");
  cJSON_AddStringToObject(header, "name", "Discover");
  cJSON_AddStringToObject(header, "payloadVersion", "3");
  cJSON_AddStringToObject(header, "messageId", msgid);

  cJSON *payload = cJSON_AddObjectToObject(directive, "header");
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

  if (curl_adapter->perform()) {
    free(data);
    data = nullptr;

    long response_code = curl_adapter->get_response_code();

    if (response_code == 200 && response_code == 202) {
      cJSON *response = cJSON_Parse(request_result.c_str());
      request_result.clear();

      if (response) {
        cJSON *event = cJSON_GetObjectItem(response, "event");
        if (event) {
          header = cJSON_GetObjectItem(event, "header");
          if (header) {
            cJSON *name = cJSON_GetObjectItem(event, "name");
            cJSON *messageId = cJSON_GetObjectItem(event, "messageId");

            if (name && cJSON_IsString(name) && messageId &&
                cJSON_IsString(messageId)) {
              st_uuid_v4(msgid);
              cJSON_SetValuestring(name, "AddOrUpdateReport");
              cJSON_SetValuestring(messageId, msgid);
              data = cJSON_PrintUnformatted(response);
            }
          }
        }
        cJSON_Delete(response);

        if (data) {
          supla_log(LOG_ERR,
                    "The received payload for Alexa Discovery is empty or in "
                    "the wrong format.");
        }
      }
    } else {
      supla_log(LOG_ERR,
                "Unsuccessful attempt to obtain a payload for Alexa Discovery. "
                "Response code: %i",
                response_code);
    }
  }

  if (data) {
    request_result = data;
    free(data);
    return request_result;
  }

  return "";
}
