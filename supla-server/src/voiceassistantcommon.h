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

#ifndef VOICEASSISTANTCOMMON_H_
#define VOICEASSISTANTCOMMON_H_

#include <sys/time.h>

class supla_user;

class supla_voice_assistant_common {
 private:
  supla_user *user;

  char *access_token;
  struct timeval set_at;

  void *lck1;
  void *lck2;
  void token_free(void);

 protected:
  virtual int get_token_maxsize(void) = 0;

 public:
  explicit supla_voice_assistant_common(supla_user *user);
  virtual ~supla_voice_assistant_common();

  int getUserID();
  supla_user *getUser();

  void set(const char *access_token);

  bool isAccessTokenExists(void);
  char *getAccessToken(void);
  struct timeval getSetTime(void);

  void data_lock(void);
  void data_unlock(void);
  void refresh_lock(void);
  void refresh_unlock(void);
};

#endif /* VOICEASSISTANTCOMMON_H_ */
