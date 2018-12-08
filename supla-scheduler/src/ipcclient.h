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

#ifndef IPCCLIENT_H_
#define IPCCLIENT_H_

#include "supla-scheduler.h"

#define IPC_BUFFER_SIZE 256

#define IPC_RESULT_SERVER_UNREACHABLE -1
#define IPC_RESULT_DISCONNECTED 0
#define IPC_RESULT_CONNECTED 1

class ipc_client {
 private:
  int sfd;
  char buffer[IPC_BUFFER_SIZE];
  int read(void);

  bool get_value(const char *cmd, int user_id, int device_id, int channel_id);
  bool check_set_result(void);

 public:
  ipc_client();
  ~ipc_client();
  bool ipc_connect(void);
  bool ipc_disconnect(void);

  char is_connected(int user_id, int device_id);

  bool get_double_value(int user_id, int device_id, int channel_id,
                        double *value);
  bool get_char_value(int user_id, int device_id, int channel_id, char *value);
  bool get_rgbw_value(int user_id, int device_id, int channel_id, int *color,
                      char *color_brightness, char *brightness);

  bool set_char_value(int user_id, int device_id, int channel_id,
                      int channel_group_id, char value);
  bool set_rgbw_value(int user_id, int device_id, int channel_id,
                      int channel_group_id, int color, char color_brightness,
                      char brightness);
};

#endif /* IPCCLIENT_H_ */
