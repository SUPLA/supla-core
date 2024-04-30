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

#include "http_event_hub.h"

#include "amazon/alexa_change_report_request.h"
#include "amazon/alexa_delete_request.h"
#include "amazon/alexa_discover_request.h"
#include "amazon/alexa_response_request.h"
#include "google/google_home_state_report_request.h"
#include "google/google_home_sync_request.h"
#include "webhook/state_webhook_request.h"

// static
void supla_http_event_hub::on_channel_value_change(supla_user *user,
                                                   int deviceId, int channelId,
                                                   const supla_caller &caller) {
  supla_alexa_change_report_request::new_request(caller, user, deviceId,
                                                 channelId);

  supla_state_webhook_request::new_request(caller, user, deviceId, channelId,
                                           0);
}

// static
void supla_http_event_hub::on_value_change_request(
    supla_user *user, int deviceId, int channelId, const supla_caller &caller,
    supla_alexa_correlation_token *correlationToken,
    const char googleRequestId[]) {
  if (correlationToken) {
    supla_alexa_response_request::new_request(caller, user, deviceId, channelId,
                                              correlationToken);
  }

  if (googleRequestId) {
    supla_google_home_state_report_request::new_request(
        caller, user, deviceId, channelId, googleRequestId);
  }
}

// static
void supla_http_event_hub::on_channel_added(supla_user *user, int deviceId,
                                            const supla_caller &caller) {
  on_voice_assistant_sync_needed(user, caller);
}

// static
void supla_http_event_hub::before_device_delete(supla_user *user, int deviceId,
                                                const supla_caller &caller) {
  supla_alexa_delete_request::new_request(user, deviceId, false);
}

// static
void supla_http_event_hub::on_device_deleted(supla_user *user, int deviceId,
                                             const supla_caller &caller) {
  supla_google_home_sync_request::new_request(user);
}

// static
void supla_http_event_hub::on_user_reconnect(supla_user *user,
                                             const supla_caller &caller) {
  on_voice_assistant_sync_needed(user, caller);
}

// static
void supla_http_event_hub::on_voice_assistant_sync_needed(
    supla_user *user, const supla_caller &caller) {
  supla_google_home_sync_request::new_request(user);
  supla_alexa_discover_request::new_request(user);
}

// static
void supla_http_event_hub::on_actions_triggered(const supla_caller &caller,
                                                supla_user *user, int deviceId,
                                                int channelId,
                                                unsigned int actions) {
  if (actions) {
    supla_state_webhook_request::new_request(caller, user, deviceId, channelId,
                                             actions);
  }
}

// static
void supla_http_event_hub::on_scene_removed(supla_user *user, int sceneId,
                                            const supla_caller &caller) {
  supla_google_home_sync_request::new_request(user);
  supla_alexa_delete_request::new_request(user, sceneId, true);
}
