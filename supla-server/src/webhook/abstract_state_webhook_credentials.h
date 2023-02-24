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

#ifndef ABSTRACT_STATE_WEBHOOK_CREDENTIALS_H_
#define ABSTRACT_STATE_WEBHOOK_CREDENTIALS_H_

#include <list>
#include <string>

class supla_abstract_state_webhook_credentials {
 public:
  supla_abstract_state_webhook_credentials(void);
  virtual ~supla_abstract_state_webhook_credentials(void);
  virtual void refresh_lock(void) = 0;
  virtual void refresh_unlock(void) = 0;
  virtual std::string get_user_short_unique_id(void) = 0;
  virtual std::string get_url(void) = 0;
  virtual std::string get_access_token(void) = 0;
  virtual std::string get_refresh_token(void) = 0;
  virtual bool is_access_token_exists(void) = 0;
  virtual bool is_refresh_token_exists(void) = 0;
  virtual int expires_in(void) = 0;
  virtual struct timeval get_set_time(void) = 0;
  virtual void update(const char *access_token, const char *refresh_token,
                      int expires_in) = 0;
  virtual void remove(void) = 0;
  virtual std::list<int> get_function_ids(void) = 0;
};

#endif /* ABSTRACT_STATE_WEBHOOK_CREDENTIALS_H_ */
