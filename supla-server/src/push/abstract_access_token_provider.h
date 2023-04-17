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

#ifndef ABSTRACT_ACCESS_TOKEN_PROVIDER_H_
#define ABSTRACT_ACCESS_TOKEN_PROVIDER_H_

#include <string>

#include "http/curl_adapter.h"

class supla_abstract_access_token_provider {
 private:
  void *data_lck;
  void *refresh_lck;
  struct timeval last_refresh_attpemt_time;
  struct timeval expires_at;
  std::string token;
  void *thread;
  supla_abstract_curl_adapter *curl_adapter;

  static void _service_loop(void *_provider, void *sthread);
  void service_loop();

 protected:
  virtual int min_secs_between_refresh_attempts(void);
  virtual int refresh_time_margin_secs(void);
  virtual long long service_tick_time_usec(void);
  virtual bool new_token(supla_abstract_curl_adapter **curl_adapter,
                         std::string *token, int *expires_in_secs) = 0;

 public:
  supla_abstract_access_token_provider(void);
  virtual ~supla_abstract_access_token_provider(void);
  virtual void start_service(void);
  void stop_service(void);

  bool refresh(supla_abstract_curl_adapter **curl_adapter);

  std::string get_token(void);
  virtual std::string get_url(void) = 0;
  bool is_token_valid(void);
  bool is_service_running(void);
};

#endif /* ABSTRACT_ACCESS_TOKEN_PROVIDER_H_ */
