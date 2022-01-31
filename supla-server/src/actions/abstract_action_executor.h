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

#include <abstract_value_getter.h>

#include "device.h"
#include "user.h"

class supla_user_channelgroups;
class supla_abstract_action_executor {
 private:
  supla_user *user;
  int device_id;
  int subject_id;
  bool is_group;

 protected:
  void execute_action(
      std::function<void(supla_user_channelgroups *, supla_device_channels *)>
          f);
  void access_device(std::function<void(supla_device *device)> on_device);
  supla_user *get_user(void);
  supla_user_channelgroups *get_channel_groups(void);

 public:
  supla_abstract_action_executor(void);
  supla_abstract_action_executor(supla_user *user, int device_id,
                                 int channel_id);
  supla_abstract_action_executor(int user_id, int device_id, int channel_id);
  supla_abstract_action_executor(supla_user *user, int group_id);
  supla_abstract_action_executor(int user_id, int group_id);
  virtual ~supla_abstract_action_executor(void);

  void set_channel_id(supla_user *user, int device_id, int channel_id);
  void set_channel_id(int user_id, int device_id, int channel_id);
  void set_group_id(supla_user *user, int group_id);
  void set_group_id(int user_id, int group_id);

  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
  int get_group_id(void);

  virtual void set_on(bool on) = 0;
  virtual void set_color(unsigned int color) = 0;
  virtual void set_brightness(char brightness) = 0;
  virtual void set_color_brightness(char brightness) = 0;
  virtual void set_rgbw(unsigned int *color, char *color_brightness,
                        char *brightness, char *on_off) = 0;
  virtual void toggle(void) = 0;
  virtual void shut(const char *closingPercentage) = 0;
  virtual void reveal(void) = 0;
  virtual void up(void) = 0;
  virtual void down(void) = 0;
  virtual void up_or_stop(void) = 0;
  virtual void down_or_stop(void) = 0;
  virtual void step_by_step(void) = 0;
  virtual void stop(void) = 0;
  virtual void open(void) = 0;
  virtual void close(void) = 0;
  virtual void open_close(void) = 0;
  virtual void open_close_without_canceling_tasks(void) = 0;
  virtual void forward_outside(int cap) = 0;
  void copy(supla_abstract_value_getter *value_getter, int sourceDeviceId,
            int sourceChannelId);
};

#endif /*ABSTRACT_ACTION_EXECUTOR_H_*/
