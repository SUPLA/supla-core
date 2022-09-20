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

#include "client/call_handler/call_handler_collection.h"

#include <call_handler/register_client_a.h>

#include "client/call_handler/auth_and_execute_action.h"
#include "client/call_handler/calcfg_request.h"
#include "client/call_handler/calcfg_request_b.h"
#include "client/call_handler/channel_set_value.h"
#include "client/call_handler/channel_set_value_b.h"
#include "client/call_handler/clients_reconnect_request.h"
#include "client/call_handler/device_reconnect_request.h"
#include "client/call_handler/execute_action.h"
#include "client/call_handler/get_channel_basic_cfg.h"
#include "client/call_handler/get_channel_state.h"
#include "client/call_handler/get_next.h"
#include "client/call_handler/oauth_token_request.h"
#include "client/call_handler/register_client_b.h"
#include "client/call_handler/register_client_c.h"
#include "client/call_handler/register_client_d.h"
#include "client/call_handler/set_caption.h"
#include "client/call_handler/set_channel_function.h"
#include "client/call_handler/set_registration_enabled.h"
#include "client/call_handler/set_value.h"
#include "client/call_handler/superuser_authorization_request.h"
#include "client/call_handler/superuser_authorization_result.h"
#include "client/call_handler/timer_arm.h"
#include "conn/call_handler/get_registration_enabled.h"
#include "conn/call_handler/get_user_localtime.h"
#include "conn/call_handler/get_version.h"
#include "conn/call_handler/ping_server.h"
#include "conn/call_handler/set_activity_timeout.h"

supla_client_call_handler_collection::supla_client_call_handler_collection(void)
    : supla_abstract_srpc_call_handler_collection() {
  add_handler(new supla_ch_set_value());
  add_handler(new supla_ch_channel_set_value_b());
  add_handler(new supla_ch_channel_set_value());
  add_handler(new supla_ch_auth_and_execute_action());
  add_handler(new supla_ch_execute_action());
  add_handler(new supla_ch_timer_arm());
  add_handler(new supla_ch_get_next());
  add_handler(new supla_ch_get_channel_state());
  add_handler(new supla_ch_calcfg_request_b());
  add_handler(new supla_ch_calcfg_request());
  add_handler(new supla_ch_oauth_token_request());
  add_handler(new supla_ch_superuser_authorization_request());
  add_handler(new supla_ch_superuser_authorization_result());
  add_handler(new supla_ch_get_channel_basic_cfg());
  add_handler(new supla_ch_set_caption());
  add_handler(new supla_ch_set_channel_function());
  add_handler(new supla_ch_clients_reconnect_request());
  add_handler(new supla_ch_set_registration_enabled());
  add_handler(new supla_ch_device_reconnect_request());
  add_handler(new supla_ch_register_client_d);
  add_handler(new supla_ch_register_client_c);
  add_handler(new supla_ch_register_client_b);
  add_handler(new supla_ch_register_client_a);

  // common
  add_handler(new supla_ch_get_version());
  add_handler(new supla_ch_set_activity_timeout());
  add_handler(new supla_ch_ping_server());
  add_handler(new supla_ch_get_user_localtime());
  add_handler(new supla_ch_get_registration_enabled());
}

supla_client_call_handler_collection::~supla_client_call_handler_collection() {}
