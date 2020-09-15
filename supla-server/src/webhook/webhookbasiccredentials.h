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

#ifndef WEBHOOK_WEBHOOKBASICCREDENTIALS_H_
#define WEBHOOK_WEBHOOKBASICCREDENTIALS_H_

#include <sys/time.h>

class supla_user;

class supla_webhook_basic_credentials {
 private:
  supla_user *user;

  char *access_token;
  char *refresh_token;

  struct timeval expires_at;
  struct timeval set_at;

  void *lck1;
  void *lck2;
  void access_token_free(void);
  void refresh_token_free(void);

 public:
  explicit supla_webhook_basic_credentials(supla_user *user);
  virtual ~supla_webhook_basic_credentials();
  virtual int get_token_maxsize(void) = 0;
  int getUserID();
  supla_user *getUser();

  void set(const char *access_token);
  void set(const char *access_token, const char *refresh_token, int expires_in);
  virtual void update(const char *access_token, const char *refresh_token,
                      int expires_in) = 0;

  bool isAccessTokenExists(void);
  bool isRefreshTokenExists(void);

  char *getAccessToken(void);
  char *getRefreshToken(void);
  struct timeval getSetTime(void);

  int expiresIn(void);

  void data_lock(void);
  void data_unlock(void);
  void refresh_lock(void);
  void refresh_unlock(void);
};

#endif /* WEBHOOK_WEBHOOKBASICCREDENTIALS_H_ */
