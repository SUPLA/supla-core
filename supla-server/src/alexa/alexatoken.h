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

#ifndef ALEXA_ALEXATOKEN_H_
#define ALEXA_ALEXATOKEN_H_

#define TOKEN_MAXSIZE 1024

#include <sys/time.h>

class supla_user;

class supla_alexa_token {
private:
	supla_user *user;

	char *token;
	char *refresh_token;
	struct timeval expires_at;
	struct timeval set_at;

	void *lck1;
	void *lck2;
	void release_strings(void);
 public:
  supla_alexa_token(supla_user *user);
  virtual ~supla_alexa_token();
  int getUserID();
  void load();
  void set(const char *token, const char *refresh_token, int expires_in);

  bool isTokenExists(void);
  bool isRefreshTokenExists(void);
  int expiresIn(void);
  char *getToken(void);
  char *getRefreshToken(void);
  struct timeval getSetTime(void);

  void refresh_lock(void);
  void refres_unlock(void);

};

#endif /* ALEXA_ALEXATOKEN_H_ */
