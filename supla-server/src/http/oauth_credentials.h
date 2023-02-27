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

#ifndef SUPLA_OAUTH_CREDENTIALS_H_
#define SUPLA_OAUTH_CREDENTIALS_H_

#include <string>

class supla_user;
class supla_http_oauth_credentials {
 private:
  void *refresh_lck;
  void *data_lck;
  supla_user *user;

  std::string access_token;
  std::string refresh_token;
  struct timeval expires_at;
  struct timeval set_at;

 protected:
  virtual void data_lock(void);
  virtual void data_unlock(void);

  virtual void set(const std::string access_token,
                   const std::string refresh_token, int expires_in);

 public:
  supla_http_oauth_credentials(void);
  supla_http_oauth_credentials(supla_user *user);
  virtual ~supla_http_oauth_credentials(void);
  virtual void refresh_lock(void);
  virtual void refresh_unlock(void);
  virtual supla_user *get_user(void);
  virtual int get_user_id(void);
  virtual std::string get_user_long_unique_id(void);
  virtual std::string get_user_short_unique_id(void);
  virtual std::string get_access_token(void);
  virtual std::string get_refresh_token(void);
  virtual bool is_access_token_exists(void);
  virtual bool is_refresh_token_exists(void);
  virtual int expires_in(void);
  virtual struct timeval get_set_time(void);
  virtual void update(const std::string access_token,
                      const std::string refresh_token, int expires_in);
  virtual void remove(void);
};

#endif /* SUPLA_OAUTH_CREDENTIALS_H_ */
