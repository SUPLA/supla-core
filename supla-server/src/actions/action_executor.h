/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.s

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef ACTION_EXECUTOR_H_
#define ACTION_EXECUTOR_H_

#include <functional>

#include "actions/abstract_action_executor.h"

class supla_action_executor : public supla_abstract_action_executor {
 private:
  void execute_action(
      std::function<void(supla_user_channelgroups *, supla_device_channels *)>
          f);

 public:
  supla_action_executor(void);
  supla_action_executor(supla_user *user, int device_id, int channel_id);
  supla_action_executor(int user_id, int device_id, int channel_id);

  virtual void set_on(bool on);
  virtual void set_color(unsigned int color);
  virtual void set_brightness(char brightness);
  virtual void set_color_brightness(char brightness);
  virtual void set_rgbw(unsigned int *color, char *color_brightness,
                        char *brightness);
  virtual void toggle(void);
  virtual void shut(const char *closingPercentage);
  virtual void reveal(void);
  virtual void stop(void);
  virtual void up(void);
  virtual void down(void);
  virtual void up_or_stop(void);
  virtual void down_or_stop(void);
  virtual void step_by_step(void);
  virtual void open(void);
  virtual void close(void);
  virtual void open_close();
  virtual void open_close_without_canceling_tasks();
  virtual void forward_outside(int cap);
};

#endif /*ACTION_EXECUTOR_H_*/
