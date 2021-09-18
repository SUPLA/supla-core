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

#ifndef ABSTRACT_ACTION_EXECUTOR_H_
#define ABSTRACT_ACTION_EXECUTOR_H_

#include "device.h"
#include "user.h"

class supla_abstract_channel_action_executor {
 private:
  supla_user *user;
  int device_id;
  int channel_id;

 protected:
  supla_device *get_device(void);
  int get_channel_id(void);

 public:
  supla_abstract_channel_action_executor(void);
  supla_abstract_channel_action_executor(supla_user *user, int device_id,
                                         int channel_id);
  supla_abstract_channel_action_executor(int user_id, int device_id,
                                         int channel_id);
  virtual ~supla_abstract_channel_action_executor(void);

  void set_channel_id(supla_user *user, int device_id, int channel_id);
  void set_channel_id(int user_id, int device_id, int channel_id);

  virtual void set_on(bool on) = 0;
  virtual void set_color(unsigned int color) = 0;
  virtual void set_brightness(char brightness) = 0;
  virtual void set_color_brightness(char brightness) = 0;
  virtual void toggle(void) = 0;
  virtual void shut(const char *closingPercentage) = 0;
  virtual void reveal(void) = 0;
  virtual void stop(void) = 0;
  virtual void open(void) = 0;
  virtual void close(void) = 0;
  virtual void open_close(void) = 0;
  virtual void open_close_without_canceling_tasks(void) = 0;
};

#endif /*ABSTRACT_ACTION_EXECUTOR_H_*/
