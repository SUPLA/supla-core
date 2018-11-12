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

#ifndef IPCCTRL_H_
#define IPCCTRL_H_

#define IPC_AUTH_TYPE_OAUTH_TOKEN 1
#define IPC_AUTH_LEVEL_UNAUTHORIZED 0
#define IPC_AUTH_LEVEL_OAUTH_USER 1
#define IPC_AUTH_LEVEL_SUPERUSER 2

#include "eh.h"

#define IPC_BUFFER_SIZE 4096

class svr_ipcctrl {
 private:
  int sfd;
  TEventHandler *eh;

  char auth_level;
  int auth_expires_at;
  int user_id;
  int oauth_user_id;

  bool match_command(const char *cmd, int len);
  void get_double(const char *cmd, char Type);
  void get_char(const char *cmd);
  void get_rgbw(const char *cmd);
  void set_char(const char *cmd, bool group);
  void set_rgbw(const char *cmd, bool group, bool random);
  void send_result(const char *result);
  void send_result(const char *result, int i);
  void send_result(const char *result, double i);

  char buffer[IPC_BUFFER_SIZE];

 public:
  explicit svr_ipcctrl(int sfd);
  void execute(void *sthread);
  virtual ~svr_ipcctrl();
};

#endif /* IPCCTRL_H_ */
