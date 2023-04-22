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

#ifndef PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_PROVIDER_H_
#define PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_PROVIDER_H_

#include <string>
#include <vector>

#include "http/curl_adapter.h"
#include "json/cJSON.h"
#include "push/pn_gateway_access_token.h"
#include "push/pn_recipients.h"

class supla_pn_gateway_access_token_provider {
 private:
  void *data_lck;
  void *refresh_lck;
  struct timeval last_refresh_attpemt_time;
  void *thread;
  std::vector<supla_pn_gateway_access_token> tokens;

  void process_result(std::vector<supla_pn_gateway_access_token> *tokens,
                      _platform_e platform, cJSON *platform_json,
                      cJSON *app_json);

  void process_result(std::vector<supla_pn_gateway_access_token> *tokens,
                      _platform_e platform, const std::string &platform_name,
                      cJSON *root);
  void get_new_tokens(std::vector<supla_pn_gateway_access_token> *tokens);

  static void _service_loop(void *_provider, void *sthread);
  void service_loop();
  static supla_pn_gateway_access_token_provider instance;

 protected:
  supla_abstract_curl_adapter *curl_adapter;

  virtual int min_secs_between_refresh_attempts(void);
  virtual int refresh_time_margin_secs(void);
  virtual long long service_tick_time_usec(void);
  bool refresh(void);

 public:
  supla_pn_gateway_access_token_provider(void);
  virtual ~supla_pn_gateway_access_token_provider(void);
  virtual void start_service(void);
  void stop_service(void);

  virtual supla_pn_gateway_access_token get_token(_platform_e platform,
                                                  int app_id);
  size_t get_token_count(void);
  bool is_any_token_available(void);
  bool is_service_running(void);

  static supla_pn_gateway_access_token_provider *global_instance(void);
};

#endif /* PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_PROVIDER_H_ */
