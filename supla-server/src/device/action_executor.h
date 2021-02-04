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

#ifndef ACTION_EXECUTOR_H_
#define ACTION_EXECUTOR_H_

#include "device.h"
#include "user.h"

class supla_action_executor {
 private:
  supla_user *user;
  int device_id;
  int channel_id;

 protected:
  supla_device *get_device(void);
 public:
  supla_action_executor(supla_user *user, int device_id, int channel_id);
  supla_action_executor(int user_id, int device_id, int channel_id);

  void set_on(bool on);
  void set_color(unsigned int color);
  void set_brightness(char brightness);
  void set_color_brightness(char brightness);
  void toggle(void);
  void shut(const char *closingPercentage);
  void reveal(void);
  void stop(void);
  void open(void);
  void open_close(void);
};

#endif /*ACTION_EXECUTOR_H_*/
