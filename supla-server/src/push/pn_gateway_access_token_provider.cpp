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

#include "pn_gateway_access_token_provider.h"

#include <unistd.h>

#include "db/database.h"
#include "db/db_access_provider.h"
#include "db/server_settings_dao.h"
#include "http/curl_adapter.h"
#include "lck.h"
#include "log.h"
#include "sthread.h"
#include "svrcfg.h"

using std::string;
using std::vector;

// static
supla_pn_gateway_access_token_provider
    supla_pn_gateway_access_token_provider::instance;

supla_pn_gateway_access_token_provider::supla_pn_gateway_access_token_provider(
    void) {
  this->curl_adapter = nullptr;
  this->refresh_lck = lck_init();
  this->data_lck = lck_init();
  this->last_refresh_attpemt_time = {};
  this->thread = nullptr;
}

supla_pn_gateway_access_token_provider::~supla_pn_gateway_access_token_provider(
    void) {
  stop_service();

  if (curl_adapter) {
    delete curl_adapter;
    curl_adapter = nullptr;
  }

  lck_free(data_lck);
  lck_free(refresh_lck);
}

long long supla_pn_gateway_access_token_provider::service_tick_time_usec(void) {
  return 1000000;
}

// static
void supla_pn_gateway_access_token_provider::_service_loop(void *_provider,
                                                           void *sthread) {
  supla_pn_gateway_access_token_provider *provider =
      static_cast<supla_pn_gateway_access_token_provider *>(_provider);
  if (!provider) {
    return;
  }

  if (!provider->curl_adapter) {
    provider->curl_adapter = new supla_curl_adapter();
  }

  while (sthread_isterminated(sthread) == 0) {
    provider->service_loop();
    usleep(provider->service_tick_time_usec());
  }

  if (provider->curl_adapter) {
    delete provider->curl_adapter;
    provider->curl_adapter = nullptr;
  }
}

void supla_pn_gateway_access_token_provider::service_loop(void) {
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  lck_lock(data_lck);
  struct timeval expires_at = {};

  for (auto it = tokens.begin(); it != tokens.end(); ++it) {
    it->get_expiration_time_if_earlier(&expires_at);
  }

  long long secs_left = expires_at.tv_sec - now.tv_sec;
  lck_unlock(data_lck);

  secs_left -= refresh_time_margin_secs();

  if (secs_left <= 0) {
    refresh();
  }
}

void supla_pn_gateway_access_token_provider::start_service(void) {
  if (thread) {
    return;
  }

  sthread_simple_run(_service_loop, this, 0, &thread);
}

void supla_pn_gateway_access_token_provider::stop_service(void) {
  if (thread) {
    sthread_twf(thread, false);
    thread = nullptr;
  }
}

int supla_pn_gateway_access_token_provider::refresh_time_margin_secs(void) {
  return 300;
}

int supla_pn_gateway_access_token_provider::min_secs_between_refresh_attempts(
    void) {
  return 60;
}

void supla_pn_gateway_access_token_provider::process_result(
    vector<supla_pn_gateway_access_token> *tokens, _platform_e platform,
    cJSON *platform_json, cJSON *app_json) {
  if (!app_json || !app_json->string) {
    return;
  }

  char *endptr = nullptr;
  int app_id = strtol(app_json->string, &endptr, 10);

  if (endptr == app_json->string || *endptr != '\0') {
    return;
  }

  cJSON *url_item = cJSON_GetObjectItem(app_json, "url");
  if (!url_item || !cJSON_IsString(url_item)) {
    return;
  }

  cJSON *devel_url_item = cJSON_GetObjectItem(app_json, "development_url");

  cJSON *token_item = cJSON_GetObjectItem(app_json, "token");
  if (!token_item || !cJSON_IsString(token_item)) {
    return;
  }

  cJSON *expires_in_item = cJSON_GetObjectItem(app_json, "expires_in");
  if (!expires_in_item || !cJSON_IsNumber(expires_in_item)) {
    return;
  }

  string bundle_id;
  if (platform == platform_ios) {
    cJSON *bundle_id_item = cJSON_GetObjectItem(app_json, "bundle_id");
    if (!bundle_id_item || !cJSON_IsString(bundle_id_item)) {
      return;
    }

    bundle_id = cJSON_GetStringValue(bundle_id_item);
    if (bundle_id.empty()) {
      return;
    }
  }

  string url = cJSON_GetStringValue(url_item);
  string devel_url = devel_url_item && cJSON_IsString(devel_url_item)
                         ? cJSON_GetStringValue(devel_url_item)
                         : "";
  string token = cJSON_GetStringValue(token_item);

  if (!url.empty() && !token.empty() && expires_in_item->valueint > 0) {
    supla_pn_gateway_access_token t(
        url, devel_url, token, expires_in_item->valueint, platform, app_id);

    if (!bundle_id.empty()) {
      t.set_extra_field("bundle_id", bundle_id);
    }

    tokens->push_back(t);
  }
}

void supla_pn_gateway_access_token_provider::process_result(
    vector<supla_pn_gateway_access_token> *tokens, _platform_e platform,
    const string &platform_name, cJSON *root) {
  cJSON *platform_json = cJSON_GetObjectItem(root, platform_name.c_str());
  if (!platform_json) {
    return;
  }

  int size = cJSON_GetArraySize(platform_json);
  for (int a = 0; a < size; a++) {
    cJSON *item = cJSON_GetArrayItem(platform_json, a);
    process_result(tokens, platform, platform_json, item);
  }
}

void supla_pn_gateway_access_token_provider::get_new_tokens(
    vector<supla_pn_gateway_access_token> *tokens) {
  curl_adapter->reset();

  string auth = "Authorization: Bearer ";

  supla_db_access_provider dba;
  supla_server_settins_dao dao(&dba);
  auth.append(dao.get_target_token());

  curl_adapter->append_header(auth.c_str());

  string url = "https://autodiscover.supla.org/pn-token";

  curl_adapter->set_opt_url(url.c_str());

  string request_result;
  curl_adapter->set_opt_write_data(&request_result);

  if (!curl_adapter->perform()) {
    return;
  }

  if (curl_adapter->get_response_code() != 200) {
    supla_log(LOG_ERR,
              "An attempt to obtain tokens for PUSH gateways from the AD "
              "server failed. Result code: %i",
              curl_adapter->get_response_code());
    return;
  }

  cJSON *root = cJSON_Parse(request_result.c_str());
  request_result = "";

  if (root) {
    process_result(tokens, platform_android, "android", root);
    process_result(tokens, platform_ios, "ios", root);

    cJSON_Delete(root);
  }

  if (!tokens->size()) {
    supla_log(LOG_ERR,
              "Could not get any token for PUSH gateway from AD server.");
  }
}

bool supla_pn_gateway_access_token_provider::refresh(void) {
  bool result = false;
  string token;

  vector<supla_pn_gateway_access_token> tokens;

  lck_lock(refresh_lck);
  struct timeval now = {};
  gettimeofday(&now, nullptr);

  if (last_refresh_attpemt_time.tv_sec == 0 ||
      now.tv_sec - last_refresh_attpemt_time.tv_sec >=
          min_secs_between_refresh_attempts()) {
    last_refresh_attpemt_time = now;

    get_new_tokens(&tokens);
  }
  lck_unlock(refresh_lck);

  if (tokens.size()) {
    lck_lock(data_lck);

    for (auto nit = tokens.begin(); nit != tokens.end(); ++nit) {
      auto it = this->tokens.begin();

      while (it != this->tokens.end()) {
        if (it->get_platform() == nit->get_platform() &&
            it->get_app_id() == nit->get_app_id()) {
          *it = *nit;
          break;
        }
        ++it;
      }

      if (it == this->tokens.end()) {
        this->tokens.push_back(*nit);
      }
    }

    lck_unlock(data_lck);
  }

  return result;
}

supla_pn_gateway_access_token supla_pn_gateway_access_token_provider::get_token(
    _platform_e platform, int app_id) {
  supla_pn_gateway_access_token result;

  lck_lock(data_lck);
  for (auto it = tokens.begin(); it != tokens.end(); ++it) {
    if (it->get_platform() == platform && it->get_app_id() == app_id) {
      result = *it;
      break;
    }
  }
  lck_unlock(data_lck);

  return result;
}

size_t supla_pn_gateway_access_token_provider::get_token_count(void) {
  lck_lock(data_lck);
  size_t result = tokens.size();
  lck_unlock(data_lck);
  return result;
}

bool supla_pn_gateway_access_token_provider::is_any_token_available(void) {
  return get_token_count() > 0;
}

bool supla_pn_gateway_access_token_provider::is_service_running(void) {
  return thread != nullptr;
}

// static
supla_pn_gateway_access_token_provider *
supla_pn_gateway_access_token_provider::global_instance(void) {
  return &instance;
}
