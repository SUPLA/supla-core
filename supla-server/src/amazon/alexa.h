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

#ifndef AMAZON_ALEXA_H_
#define AMAZON_ALEXA_H_

#define TOKEN_MAXSIZE 1024
#define REGION_MAXSIZE 5
#define ENDPOINTSCOPE_MAXSIZE 32

#include <sys/time.h>

class supla_user;

class supla_amazon_alexa {
 private:
  supla_user *user;

  char *access_token;
  char *refresh_token;
  char *region;
  char *endpoint_scope;
  struct timeval expires_at;
  struct timeval set_at;

  void *lck1;
  void *lck2;
  void release_strings(void);

 public:
  supla_amazon_alexa(supla_user *user);
  virtual ~supla_amazon_alexa();
  int getUserID();
  void load();
  void remove();
  void update(const char *access_token, const char *refresh_token,
              int expires_in);
  void set(const char *access_token, const char *refresh_token, int expires_in,
           const char *region, const char *endpoint_scope);

  bool isAccessTokenExists(void);
  bool isRefreshTokenExists(void);
  int expiresIn(void);
  char *getAccessToken(void);
  char *getRefreshToken(void);
  char *getRegion(void);
  char *getEndpointScope(void);
  struct timeval getSetTime(void);

  void refresh_lock(void);
  void refresh_unlock(void);
};

#endif /* AMAZON_ALEXA_H_ */
