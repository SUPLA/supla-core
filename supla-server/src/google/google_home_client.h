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

#ifndef GOOGLE_HOME_CLIENT_H_
#define GOOGLE_HOME_CLIENT_H_

#include <google/google_home_credentials.h>

#include <string>

#include "http/voice_assistant_client2.h"
#include "json/cJSON.h"

class supla_google_home_client : public supla_voice_assistant_client2 {
 private:
  cJSON *json_states;
  std::string request_id;

  supla_google_home_credentials *get_gh_credentials(void);
  bool perform_post_request(cJSON *json_data, int *http_result_code);
  cJSON *get_header(void);
  bool channel_exists(const char *endpoint_id);
  cJSON *get_state_skeleton(void);
  void add_open_percent_state(short open_percent);

 public:
  explicit supla_google_home_client(int channel_id,
                                    supla_abstract_curl_adapter *curl_adapter,
                                    supla_google_home_credentials *credentials);
  virtual ~supla_google_home_client(void);

  void set_request_id(const std::string &request_id);
  void add_onoff_state(void);
  void add_brightness_state(void);
  void add_color_state(void);
  void add_gate_state(void);
  void add_roller_shutter_state(void);
  bool state_report(void);
  bool sync(void);
};

#endif /* GOOGLE_HOME_CLIENT_H_ */
