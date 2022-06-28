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

#define IPC_BUFFER_SIZE 4096

#include <stddef.h>

#include <functional>
#include <vector>

#include "eh.h"
#include "ipc/abstract_ipc_command.h"

class supla_user_channelgroups;
class supla_device_channels;
class supla_ipc_ctrl {
 private:
  int sfd;
  TEventHandler *eh;

  char *AlexaCorrelationToken = NULL;
  char *GoogleRequestId = NULL;

  void channel_action(const char *cmd,
                      std::function<bool(supla_device_channels *, int)> f);
  void channel_groups_action(
      const char *cmd, std::function<bool(supla_user_channelgroups *, int)> f);

  bool match_command(const char *cmd, int len);
  char *cut(const char *cmd, const char *var);
  void free_correlation_token();
  void cut_correlation_token(const char *cmd);
  void free_google_requestid();
  void cut_google_requestid(const char *cmd);
  void get_double(const char *cmd, char Type);
  void get_char(const char *cmd);
  void get_rgbw(const char *cmd);
  void get_impulsecounter_value(const char *cmd);
  void get_electricitymeter_value(const char *cmd);
  void get_valve_value(const char *cmd);
  void get_digiglass_value(const char *cmd);
  void set_char(const char *cmd, bool group);
  void set_rgbw(const char *cmd, bool group, bool random);
  void set_digiglass_value(const char *cmd);
  void get_relay_value(const char *cmd);
  void action_cg_open_close(const char *cmd, bool open);
  void action_open_close(const char *cmd, bool open);
  void action_toggle(const char *cmd);
  void action_cg_toggle(const char *cmd);
  void action_stop(const char *cmd);
  void action_cg_stop(const char *cmd);
  void action_up_or_stop(const char *cmd);
  void action_cg_up_or_stop(const char *cmd);
  void action_down_or_stop(const char *cmd);
  void action_cg_down_or_stop(const char *cmd);
  void action_step_by_step(const char *cmd);
  void action_cg_step_by_step(const char *cmd);
  void action_copy(const char *cmd, bool group);
  void action_copy(const char *cmd);
  void action_cg_copy(const char *cmd);
  void alexa_credentials_changed(const char *cmd);
  void google_home_credentials_changed(const char *cmd);
  void state_webhook_changed(const char *cmd);
  void mqtt_settings_changed(const char *cmd);
  void before_channel_function_change(const char *cmd);
  void before_device_delete(const char *cmd);
  void on_device_deleted(const char *cmd);
  void on_device_settings_changed(const char *cmd);
  void reset_counters(const char *cmd);
  void recalibrate(const char *cmd);
  void enter_cfg_mode(const char *cmd);
  void get_status(void);
  void execute_scene(const char *cmd);
  void interrupt_scene(const char *cmd);

  void send_result(const char *result);
  void send_result(const char *result, int i);
  void send_result(const char *result, double i);

  char buffer[IPC_BUFFER_SIZE];

 public:
  explicit supla_ipc_ctrl(int sfd);
  void execute(void *sthread);
  virtual ~supla_ipc_ctrl();
};

#endif /* IPCCTRL_H_ */
