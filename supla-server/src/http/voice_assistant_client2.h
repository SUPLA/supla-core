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

#ifndef VOICE_ASSISTANT_CLIENT2_H_
#define VOICE_ASSISTANT_CLIENT2_H_

#include <google/google_home_credentials.h>
#include <string>

#include "device/value/channel_value.h"
#include "http/abstract_curl_adapter.h"

class supla_voice_assistant_client2 {
 private:
  int channel_id;
  short subchannel_id;
  bool channel_connected;
  supla_channel_value *channel_value;
  supla_abstract_curl_adapter *curl_adapter;
  supla_http_oauth_credentials *credentials;

 protected:
  int get_channel_id(void);
  short get_subchannel_id(void);
  supla_channel_value *get_channel_value(void);
  supla_abstract_curl_adapter *get_curl_adapter(void);
  supla_http_oauth_credentials *get_credentials(void);
  bool is_channel_connected(void);
  std::string get_endpoint_id(void);

 public:
  explicit supla_voice_assistant_client2(
      int channel_id, supla_abstract_curl_adapter *curl_adapter,
      supla_http_oauth_credentials *credentials);
  void set_subchannel_id(short subchannel_id);
  void set_channel_connected(bool connected);
  void set_channel_value(supla_channel_value *channel_value);
};

#endif /* VOICE_ASSISTANT_CLIENT2_H_ */
