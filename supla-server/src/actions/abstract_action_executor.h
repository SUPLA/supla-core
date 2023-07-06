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

#include <map>
#include <memory>
#include <string>

#include "abstract_action_config.h"
#include "caller.h"
#include "device.h"
#include "device/abstract_channel_property_getter.h"
#include "user.h"

class supla_user_channelgroups;
class supla_abstract_action_executor {
 private:
  supla_caller caller;
  supla_user *user;
  int device_id;
  int subject_id;
  _subjectType_e subject_type;

 protected:
  void execute_action(
      std::function<void(supla_user_channelgroups *, supla_device_channels *)>
          f);
  std::shared_ptr<supla_device> get_device(void);
  supla_user *get_user(void);
  supla_user_channelgroups *get_channel_groups(void);
  void set_unknown_subject_type(void);

 public:
  supla_abstract_action_executor(void);
  virtual ~supla_abstract_action_executor(void);

  void set_caller(const supla_caller &caller);
  void set_channel_id(supla_user *user, int device_id, int channel_id);
  void set_channel_id(int user_id, int device_id, int channel_id);
  void set_group_id(supla_user *user, int group_id);
  void set_group_id(int user_id, int group_id);
  void set_scene_id(supla_user *user, int scene_id);
  void set_scene_id(int user_id, int scene_id);
  void set_schedule_id(supla_user *user, int schedule_id);
  void set_schedule_id(int user_id, int schedule_id);
  void set_push_notification_id(supla_user *user, int push_id);
  void set_push_notification_id(int user_id, int push_id);

  const supla_caller &get_caller(void);
  int get_user_id(void);
  int get_device_id(void);
  int get_channel_id(void);
  int get_group_id(void);
  int get_scene_id(void);
  int get_schedule_id(void);
  int get_push_notification_id(void);

  void execute_action(const supla_caller &caller, int user_id,
                      abstract_action_config *config,
                      supla_abstract_channel_property_getter *property_getter);

  void execute_action(const supla_caller &caller, int user_id,
                      abstract_action_config *config,
                      supla_abstract_channel_property_getter *property_getter,
                      std::map<std::string, std::string> *replacement_map);

  void execute_action(const supla_caller &caller, int user_id, int action_id,
                      _subjectType_e subject_type, int subject_id,
                      supla_abstract_channel_property_getter *property_getter,
                      TAction_RS_Parameters *rs, TAction_RGBW_Parameters *rgbw,
                      int source_device_id, int source_channel_id, int cap,
                      std::map<std::string, std::string> *replacement_map);

  virtual void set_on(bool on) = 0;
  virtual void set_color(unsigned int color) = 0;
  virtual void set_brightness(char brightness) = 0;
  virtual void set_color_brightness(char brightness) = 0;
  virtual void set_rgbw(unsigned int *color, char *color_brightness,
                        char *brightness, char *on_off) = 0;
  virtual void toggle(void) = 0;
  virtual void shut(const char *closingPercentage, bool delta) = 0;
  virtual void reveal(void) = 0;
  virtual void up(void) = 0;
  virtual void down(void) = 0;
  virtual void up_or_stop(void) = 0;
  virtual void down_or_stop(void) = 0;
  virtual void step_by_step(void) = 0;
  virtual void enable(void) = 0;
  virtual void disable(void) = 0;
  virtual void send(std::map<std::string, std::string> *replacement_map) = 0;
  virtual void execute(void) = 0;
  virtual void interrupt(void) = 0;
  virtual void interrupt_and_execute(void) = 0;
  virtual void stop(void) = 0;
  virtual void open(void) = 0;
  virtual void close(void) = 0;
  virtual void open_close(void) = 0;
  virtual void open_close_without_canceling_tasks(void) = 0;
  virtual void forward_outside(int cap) = 0;
  void copy(supla_abstract_channel_property_getter *property_getter,  // NOLINT
            int source_device_id, int source_channel_id);             // NOLINT
};

#endif /*ABSTRACT_ACTION_EXECUTOR_H_*/
