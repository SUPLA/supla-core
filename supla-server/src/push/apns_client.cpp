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

#include "apns_client.h"

#include <list>

#include "json/cJSON.h"
#include "log.h"

using std::list;
using std::string;

supla_apns_client::supla_apns_client(
    const supla_caller &caller, supla_abstract_curl_adapter *curl_adapter,
    supla_remote_gateway_access_token_provider *token_provider,
    supla_push_notification *push)
    : supla_abstract_pn_gateway_client(caller, curl_adapter, token_provider,
                                       push) {}

supla_apns_client::~supla_apns_client(void) {}

_platform_e supla_apns_client::get_platform(void) { return platform_push_ios; }

char *supla_apns_client::get_payload(supla_pn_recipient *recipient) {
  cJSON *payload = cJSON_CreateObject();
  cJSON *aps = cJSON_CreateObject();
  cJSON *alert = cJSON_CreateObject();

  if (!get_push_notification()->get_title().empty()) {
    cJSON_AddStringToObject(alert, "title",
                            get_push_notification()->get_title().c_str());
  }

  if (!get_push_notification()->get_body().empty()) {
    cJSON_AddStringToObject(alert, "body",
                            get_push_notification()->get_body().c_str());
  }

  if (!get_push_notification()->get_localized_title().empty()) {
    cJSON_AddStringToObject(
        alert, "title-loc-key",
        get_push_notification()->get_localized_title().c_str());
  }

  if (get_push_notification()->get_localized_title_args().size()) {
    add_args(get_push_notification()->get_localized_title_args(),
             "title-loc-args", alert, false);
  }

  if (!get_push_notification()->get_localized_body().empty()) {
    cJSON_AddStringToObject(
        alert, "loc-key",
        get_push_notification()->get_localized_body().c_str());
  }

  if (get_push_notification()->get_localized_body_args().size()) {
    add_args(get_push_notification()->get_localized_body_args(), "loc-args",
             alert, false);
  }

  cJSON_AddItemToObject(payload, "aps", aps);

  if (!recipient->get_profile_name().empty()) {
    cJSON_AddStringToObject(payload, "profileName",
                            recipient->get_profile_name().c_str());
  }

  if (get_caller().get_id()) {
    if (get_caller() == ctDevice) {
      cJSON_AddNumberToObject(payload, "deviceId", get_caller().get_id());
    } else if (get_caller() == ctChannel) {
      cJSON_AddNumberToObject(payload, "channelId", get_caller().get_id());
    }
  }

  cJSON_AddItemToObject(aps, "alert", alert);
  cJSON_AddStringToObject(aps, "sound", "default");
  cJSON_AddNumberToObject(aps, "content-available", 1);

  char *result = cJSON_PrintUnformatted(payload);
  cJSON_Delete(payload);
  return result;
}

bool supla_apns_client::_send(supla_remote_gateway_access_token *token,
                              supla_pn_recipient *recipient) {
  get_curl_adapter()->append_header(recipient->get_app_id(),
                                    "content-type: application/json");

  string _token = "authorization: bearer " + token->get_token();
  get_curl_adapter()->append_header(recipient->get_app_id(), _token.c_str());

  string topic = "apns-topic: " + token->get_extra_field("bundle_id");
  get_curl_adapter()->append_header(recipient->get_app_id(), topic.c_str());

  get_curl_adapter()->append_header(recipient->get_app_id(),
                                    "apns-push-type: alert");
  get_curl_adapter()->append_header(recipient->get_app_id(),
                                    "apns-priority: 10");

  string endpoint = token->get_url(recipient->is_development_env());
  size_t pos = endpoint.find("{device_token}");
  if (pos != std::string::npos) {
    endpoint.replace(pos, std::string("{device_token}").length(),
                     get_curl_adapter()->escape(recipient->get_app_id(),
                                                recipient->get_token()));
  } else {
    supla_log(LOG_ERR, "Url %s does not contain the string {device_token}",
              endpoint.c_str());
    return false;
  }

  get_curl_adapter()->set_opt_url(recipient->get_app_id(), endpoint.c_str());

  char *payload = get_payload(recipient);
  get_curl_adapter()->set_opt_post_fields(recipient->get_app_id(), payload);

  string request_result;
  list<string> header;
  get_curl_adapter()->set_opt_write_data(recipient->get_app_id(),
                                         &request_result);
  get_curl_adapter()->set_opt_header_data(recipient->get_app_id(), &header);

  bool result = get_curl_adapter()->perform(recipient->get_app_id());
  free(payload);

  if (result) {
    result =
        get_curl_adapter()->get_response_code(recipient->get_app_id()) == 200;
    if (!result) {
      string reason;

      if (get_curl_adapter()->get_response_code(recipient->get_app_id()) ==
          400) {
        cJSON *response_json = cJSON_Parse(request_result.c_str());
        if (response_json) {
          cJSON *reason_json = cJSON_GetObjectItem(response_json, "reason");
          if (reason_json && cJSON_IsString(reason_json)) {
            reason = cJSON_GetStringValue(reason_json);
          }
          cJSON_Delete(response_json);
        }
      }

      if (reason == "BadDeviceToken" || get_curl_adapter()->get_response_code(
                                            recipient->get_app_id()) == 410) {
        recipient->set_exists(false);
      } else {
        supla_log(
            LOG_ERR, "The APNs server returned an unexpected code: %i%s%s",
            get_curl_adapter()->get_response_code(recipient->get_app_id()),
            reason.empty() ? "" : ", reason: ",
            reason.empty() ? "" : reason.c_str());
      }

    } else {
      string prefix = "apns-id: ";

      for (auto it = header.begin(); it != header.end(); ++it) {
        size_t pos = it->find(prefix);
        if (pos != std::string::npos && pos == 0) {
          recipient->set_message_id(it->substr(pos + prefix.length()));
          break;
        }
      }
    }
  }

  return result;
}
