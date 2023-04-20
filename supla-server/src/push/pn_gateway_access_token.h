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

#ifndef PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_H_
#define PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_H_

#include <string>

#include "push/pn_recipients.h"

class supla_pn_gateway_access_token {
 private:
  struct timeval expires_at;
  std::string url;
  std::string token;
  _app_id_e app_id;
  _platform_e platform;

 public:
  supla_pn_gateway_access_token(void);
  supla_pn_gateway_access_token(const std::string &url,
                                const std::string &token, int expires_in,
                                _platform_e platform, _app_id_e app_id);
  virtual ~supla_pn_gateway_access_token(void);

  _platform_e get_platform(void);
  _app_id_e get_app_id(void);
  std::string get_url(void);
  std::string get_token(void);
  void set_token(const std::string &token);
  bool is_valid(void);
  void get_expiration_time_if_earlier(struct timeval *expires_at);
};

#endif /* PUSH_NOTIFICATION_GATEWAY_ACCESS_TOKEN_H_ */
