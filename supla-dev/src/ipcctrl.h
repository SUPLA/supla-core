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

#include "eh.h"

class svr_ipcctrl {
 private:
  int sfd;
  TEventHandler *eh;
  bool match_command(const char *cmd, int len);
  void send_result(const char *result);
  void send_result(const char *result, int i);
  void send_result(const char *result, double i);
  char buffer[256];

 public:
  explicit svr_ipcctrl(int sfd);
  void execute(void *sthread);
  virtual ~svr_ipcctrl();
};

#endif /* IPCCTRL_H_ */
