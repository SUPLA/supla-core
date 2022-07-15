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

#include "sthread.h"

// TODO(anyone): For setters, use the supla_action_executor class

const char hello[] = "SUPLA SERVER CTRL\n";
const char cmd_is_client_connected[] = "IS-CLIENT-CONNECTED:";
const char cmd_is_iodev_connected[] = "IS-IODEV-CONNECTED:";
const char cmd_is_channel_connected[] = "IS-CHANNEL-CONNECTED:";
const char cmd_is_channel_online[] = "IS-CHANNEL-ONLINE:";
const char cmd_user_reconnect[] = "USER-RECONNECT:";
const char cmd_client_reconnect[] = "CLIENT-RECONNECT:";
const char cmd_get_double_value[] = "GET-DOUBLE-VALUE:";
const char cmd_get_temperature_value[] = "GET-TEMPERATURE-VALUE:";
const char cmd_get_humidity_value[] = "GET-HUMIDITY-VALUE:";
const char cmd_get_char_value[] = "GET-CHAR-VALUE:";
const char cmd_get_rgbw_value[] = "GET-RGBW-VALUE:";
const char cmd_get_em_value[] = "GET-EM-VALUE:";
const char cmd_get_ic_value[] = "GET-IC-VALUE:";
const char cmd_get_valve_value[] = "GET-VALVE-VALUE:";
const char cmd_get_relay_value[] = "GET-RELAY-VALUE:";

const char cmd_set_char_value[] = "SET-CHAR-VALUE:";
const char cmd_set_rgbw_value[] = "SET-RGBW-VALUE:";
const char cmd_set_rand_rgbw_value[] = "SET-RAND-RGBW-VALUE:";

const char cmd_set_cg_char_value[] = "SET-CG-CHAR-VALUE:";
const char cmd_set_cg_rgbw_value[] = "SET-CG-RGBW-VALUE:";
const char cmd_set_cg_rand_rgbw_value[] = "SET-CG-RAND-RGBW-VALUE:";

const char cmd_set_digiglass_value[] = "SET-DIGIGLASS-VALUE:";
const char cmd_get_digiglass_value[] = "GET-DIGIGLASS-VALUE:";

const char cmd_action_open[] = "ACTION-OPEN:";
const char cmd_action_cg_open[] = "ACTION-CG-OPEN:";
const char cmd_action_close[] = "ACTION-CLOSE:";
const char cmd_action_cg_close[] = "ACTION-CG-CLOSE:";
const char cmd_action_toggle[] = "ACTION-TOGGLE:";
const char cmd_action_cg_toggle[] = "ACTION-CG-TOGGLE:";
const char cmd_action_stop[] = "ACTION-STOP:";
const char cmd_action_cg_stop[] = "ACTION-CG-STOP:";
const char cmd_action_copy[] = "ACTION-COPY:";
const char cmd_action_cg_copy[] = "ACTION-CG-COPY:";

const char cmd_action_up_or_stop[] = "ACTION-UP-OR-STOP:";
const char cmd_action_cg_up_or_stop[] = "ACTION-CG-UP-OR-STOP:";
const char cmd_action_down_or_stop[] = "ACTION-DOWN-OR-STOP:";
const char cmd_action_cg_down_or_stop[] = "ACTION-CG-DOWN-OR-STOP:";
const char cmd_action_step_by_step[] = "ACTION-SBS:";
const char cmd_action_cg_step_by_step[] = "ACTION-CG-SBS:";

const char cmd_reset_counters[] = "RESET-COUNTERS:";
const char cmd_recalibrate[] = "RECALIBRATE:";
const char cmd_get_status[] = "GET-STATUS";
const char cmd_enter_cfg_mode[] = "ACTION-ENTER-CONFIGURATION-MODE:";

const char cmd_execute_scene[] = "EXECUTE-SCENE:";
const char cmd_interrupt_scene[] = "INTERRUPT-SCENE:";

const char cmd_get_scene_summary[] = "GET-SCENE-SUMMARY:";
const char cmd_on_scene_added[] = "USER-ON-SCENE-ADDED:";
const char cmd_on_scene_deleted[] = "USER-ON-SCENE-DELETED:";
const char cmd_on_scene_changed[] = "USER-ON-SCENE-CHANGED:";

const char cmd_user_alexa_credentials_changed[] =
    "USER-ALEXA-CREDENTIALS-CHANGED:";

const char cmd_user_google_home_credentials_changed[] =
    "USER-GOOGLE-HOME-CREDENTIALS-CHANGED:";

const char cmd_user_state_webhook_changed[] = "USER-STATE-WEBHOOK-CHANGED:";

const char cmd_user_on_device_deleted[] = "USER-ON-DEVICE-DELETED:";

const char cmd_user_mqtt_settings_changed[] = "USER-MQTT-SETTINGS-CHANGED:";

const char cmd_user_before_device_delete[] = "USER-BEFORE-DEVICE-DELETE:";

const char cmd_user_on_device_settings_changed[] =
    "USER-ON-DEVICE-SETTINGS-CHANGED:";

#include "ipc/before_channel_function_change_command.h"

supla_ipc_ctrl::supla_ipc_ctrl(
    supla_abstract_ipc_socket_adapter *socket_adapter)
    : supla_abstract_ipc_ctrl(socket_adapter) {
  sthread = NULL;
  add_command(new supla_before_channel_function_change_command(socket_adapter));
}

supla_ipc_ctrl::~supla_ipc_ctrl() {}

void supla_ipc_ctrl::execute(void *sthread) {
  this->sthread = sthread;
  supla_abstract_ipc_ctrl::execute();
}

bool supla_ipc_ctrl::is_terminated(void) {
  return sthread_isterminated(sthread) != 0;
}

void supla_ipc_ctrl::terminate(void) { sthread_terminate(sthread); }
