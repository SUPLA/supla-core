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

#include "ipc/ipc_ctrl.h"

// TODO(anyone): For setters, use the supla_action_executor class

#include "ipc/action_cg_command.h"
#include "ipc/action_command.h"
#include "ipc/alexa_cred_changed_command.h"
#include "ipc/before_channel_function_change_command.h"
#include "ipc/before_device_delete_command.h"
#include "ipc/client_reconnect_command.h"
#include "ipc/enter_cfg_mode_command.h"
#include "ipc/execute_scene_command.h"
#include "ipc/get_char_command.h"
#include "ipc/get_digiglass_value_command.h"
#include "ipc/get_double_command.h"
#include "ipc/get_em_value_command.h"
#include "ipc/get_humidity_command.h"
#include "ipc/get_ic_value_command.h"
#include "ipc/get_pn_limit_command.h"
#include "ipc/get_relay_value_command.h"
#include "ipc/get_rgbw_command.h"
#include "ipc/get_scene_summary_command.h"
#include "ipc/get_status_command.h"
#include "ipc/get_temperature_command.h"
#include "ipc/get_valve_value_command.h"
#include "ipc/gh_cred_changed_command.h"
#include "ipc/interrupt_and_execute_scene_command.h"
#include "ipc/interrupt_scene_command.h"
#include "ipc/is_channel_connected_command.h"
#include "ipc/is_client_connected_command.h"
#include "ipc/is_device_connected_command.h"
#include "ipc/on_device_deleted_command.h"
#include "ipc/on_device_settings_changed_command.h"
#include "ipc/on_mqtt_settings_changed_command.h"
#include "ipc/on_scene_added_command.h"
#include "ipc/on_scene_changed_command.h"
#include "ipc/on_scene_removed_command.h"
#include "ipc/on_state_webhook_changed_command.h"
#include "ipc/on_user_settings_changed_command.h"
#include "ipc/on_vbt_changed_command.h"
#include "ipc/recalibrate_command.h"
#include "ipc/reset_counters_command.h"
#include "ipc/send_push_command.h"
#include "ipc/set_cg_char_command.h"
#include "ipc/set_cg_rgbw_command.h"
#include "ipc/set_char_command.h"
#include "ipc/set_digiglass_value_command.h"
#include "ipc/set_rgbw_command.h"
#include "ipc/user_reconnect_command.h"
#include "sthread.h"

supla_ipc_ctrl::supla_ipc_ctrl(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_ctrl(socket_adapter) {
  sthread = NULL;

  add_command(new supla_is_client_connected_command(socket_adapter));
  add_command(new supla_is_device_connected_command(socket_adapter));
  add_command(new supla_is_channel_connected_command(socket_adapter));
  add_command(new supla_user_reconnect_command(socket_adapter));
  add_command(new supla_client_reconnect_command(socket_adapter));
  add_command(new supla_get_double_command(socket_adapter));
  add_command(new supla_get_temperature_command(socket_adapter));
  add_command(new supla_get_humidity_command(socket_adapter));
  add_command(new supla_get_char_command(socket_adapter));
  add_command(new supla_get_rgbw_command(socket_adapter));
  add_command(new supla_get_em_value_command(socket_adapter));
  add_command(new supla_get_ic_value_command(socket_adapter));
  add_command(new supla_get_valve_value_command(socket_adapter));
  add_command(new supla_get_relay_value_command(socket_adapter));
  add_command(new supla_set_char_command(socket_adapter));
  add_command(new supla_set_cg_char_command(socket_adapter));
  add_command(new supla_set_rgbw_command(socket_adapter, true));
  add_command(new supla_set_rgbw_command(socket_adapter, false));
  add_command(new supla_set_cg_rgbw_command(socket_adapter, true));
  add_command(new supla_set_cg_rgbw_command(socket_adapter, false));
  add_command(new supla_set_digiglass_value_command(socket_adapter));
  add_command(new supla_get_digiglass_value_command(socket_adapter));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_OPEN));
  add_command(new supla_action_command(socket_adapter, ACTION_OPEN));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_CLOSE));
  add_command(new supla_action_command(socket_adapter, ACTION_CLOSE));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_TURN_ON));
  add_command(new supla_action_command(socket_adapter, ACTION_TURN_ON));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_TURN_OFF));
  add_command(new supla_action_command(socket_adapter, ACTION_TURN_OFF));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_TOGGLE));
  add_command(new supla_action_command(socket_adapter, ACTION_TOGGLE));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_STOP));
  add_command(new supla_action_command(socket_adapter, ACTION_STOP));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_COPY));
  add_command(new supla_action_command(socket_adapter, ACTION_COPY));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_UP_OR_STOP));
  add_command(new supla_action_command(socket_adapter, ACTION_UP_OR_STOP));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_DOWN_OR_STOP));
  add_command(new supla_action_command(socket_adapter, ACTION_DOWN_OR_STOP));
  add_command(new supla_action_cg_command(socket_adapter, ACTION_STEP_BY_STEP));
  add_command(new supla_action_command(socket_adapter, ACTION_STEP_BY_STEP));
  add_command(new supla_reset_counters_command(socket_adapter));
  add_command(new supla_recalibrate_command(socket_adapter));
  add_command(new supla_get_status_command(socket_adapter));
  add_command(new supla_enter_cfg_mode_command(socket_adapter));
  add_command(new supla_execute_scene_command(socket_adapter));
  add_command(new supla_interrupt_scene_command(socket_adapter));
  add_command(new supla_interrupt_and_execute_scene_command(socket_adapter));
  add_command(new supla_alexa_cred_changed_command(socket_adapter));
  add_command(new supla_gh_cred_changed_command(socket_adapter));
  add_command(new supla_on_scene_added_command(socket_adapter));
  add_command(new supla_on_scene_changed_command(socket_adapter));
  add_command(new supla_on_scene_removed_command(socket_adapter));
  add_command(new supla_on_state_webhook_changed_command(socket_adapter));
  add_command(new supla_get_scene_summary_command(socket_adapter));
  add_command(new supla_on_device_deleted_command(socket_adapter));
  add_command(new supla_on_mqtt_settings_changed_command(socket_adapter));
  add_command(new supla_before_device_delete_command(socket_adapter));
  add_command(new supla_on_device_settings_changed_command(socket_adapter));
  add_command(new supla_on_user_settings_changed_command(socket_adapter));
  add_command(new supla_before_channel_function_change_command(socket_adapter));
  add_command(new supla_get_pn_limit_command(socket_adapter));
  add_command(new supla_on_vbt_changed_command(socket_adapter));
  add_command(new supla_send_push_command(socket_adapter));
}

supla_ipc_ctrl::~supla_ipc_ctrl() {}

void supla_ipc_ctrl::execute(void *sthread) {
  this->sthread = sthread;
  supla_abstract_ipc_ctrl::execute();
}

bool supla_ipc_ctrl::is_terminated(void) {
  return sthread_isterminated(sthread) != 0;
}

void supla_ipc_ctrl::terminate(void) { sthread_terminate(sthread, true); }
