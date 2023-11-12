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

#include "device/call_handler/call_handler_collection.h"

#include "conn/call_handler/get_registration_enabled.h"
#include "conn/call_handler/get_user_localtime.h"
#include "conn/call_handler/get_version.h"
#include "conn/call_handler/ping_server.h"
#include "conn/call_handler/set_activity_timeout.h"
#include "device/call_handler/action_trigger.h"
#include "device/call_handler/channel_set_value_result.h"
#include "device/call_handler/channel_state_result.h"
#include "device/call_handler/device_calcfg_result.h"
#include "device/call_handler/device_channel_extendedvalue_changed.h"
#include "device/call_handler/device_channel_value_changed.h"
#include "device/call_handler/device_channel_value_changed_b.h"
#include "device/call_handler/device_channel_value_changed_c.h"
#include "device/call_handler/get_channel_config.h"
#include "device/call_handler/get_channel_functions.h"
#include "device/call_handler/get_firmware_update_url.h"
#include "device/call_handler/register_device_a.h"
#include "device/call_handler/register_device_b.h"
#include "device/call_handler/register_device_c.h"
#include "device/call_handler/register_device_d.h"
#include "device/call_handler/register_device_e.h"
#include "device/call_handler/register_push_notification.h"
#include "device/call_handler/send_push_notification.h"
#include "device/call_handler/set_channel_config.h"
#include "device/call_handler/set_channel_config_result.h"
#include "device/call_handler/set_device_config.h"
#include "device/call_handler/set_device_config_result.h"

supla_device_call_handler_collection::supla_device_call_handler_collection(void)
    : supla_abstract_srpc_call_handler_collection() {
  add_handler(new supla_ch_action_trigger());
  add_handler(new supla_ch_channel_set_value_result());
  add_handler(new supla_ch_device_channel_value_changed_c());
  add_handler(new supla_ch_device_channel_value_changed_b());
  add_handler(new supla_ch_device_channel_value_changed());
  add_handler(new supla_ch_device_channel_extendedvalue_changed());
  add_handler(new supla_ch_channel_state_result());
  add_handler(new supla_ch_get_firmware_update_url());
  add_handler(new supla_ch_device_calcfg_result());
  add_handler(new supla_ch_get_channel_functions());
  add_handler(new supla_ch_get_channel_config());
  add_handler(new supla_ch_register_device_e);
  add_handler(new supla_ch_register_device_d);
  add_handler(new supla_ch_register_device_c);
  add_handler(new supla_ch_register_device_b);
  add_handler(new supla_ch_register_device_a);
  add_handler(new supla_ch_set_device_config());
  add_handler(new supla_ch_set_device_config_result());
  add_handler(new supla_ch_set_channel_config());
  add_handler(new supla_ch_set_channel_config_result());
  add_handler(new supla_ch_register_push_notification);
  add_handler(new supla_ch_send_push_notification);

  // common
  add_handler(new supla_ch_get_version());
  add_handler(new supla_ch_set_activity_timeout());
  add_handler(new supla_ch_ping_server());
  add_handler(new supla_ch_get_user_localtime());
  add_handler(new supla_ch_get_registration_enabled());
}

supla_device_call_handler_collection::~supla_device_call_handler_collection() {}
