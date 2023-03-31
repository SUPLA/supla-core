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

#include "http/httprequestqueue.h"

#include "amazon/alexa_change_report_request.h"
#include "amazon/alexa_response_request.h"
#include "google/google_home_state_report_request.h"
#include "google/google_home_sync_request2.h"
#include "webhook/state_webhook_request.h"

// static
void supla_http_request_queue::onChannelValueChangeEvent(
    supla_user *user, int deviceId, int channelId, const supla_caller &caller,
    const char correlationToken[], const char googleRequestId[]) {
  supla_alexa_response_request2::new_request(
      caller, user, deviceId, channelId,
      correlationToken ? correlationToken : "");

  supla_alexa_change_report_request2::new_request(caller, user, deviceId,
                                                  channelId);

  supla_google_home_state_report_request::new_request(
      caller, user, deviceId, channelId,
      googleRequestId ? googleRequestId : "");

  supla_state_webhook_request::new_request(caller, user, deviceId, channelId,
                                           0);
}

// static
void supla_http_request_queue::onChannelsAddedEvent(
    supla_user *user, int deviceId, const supla_caller &caller) {
  supla_google_home_sync_request2::new_request(user);
}

// static
void supla_http_request_queue::onDeviceDeletedEvent(
    supla_user *user, int deviceId, const supla_caller &caller) {
  supla_google_home_sync_request2::new_request(user);
}

// static
void supla_http_request_queue::onUserReconnectEvent(
    supla_user *user, const supla_caller &caller) {
  supla_google_home_sync_request2::new_request(user);
}

// static
void supla_http_request_queue::onGoogleHomeSyncNeededEvent(
    supla_user *user, const supla_caller &caller) {
  supla_google_home_sync_request2::new_request(user);
}

// static
void supla_http_request_queue::onActionsTriggered(const supla_caller &caller,
                                                  supla_user *user,
                                                  int deviceId, int channelId,
                                                  unsigned int actions) {
  if (actions) {
    supla_state_webhook_request::new_request(caller, user, deviceId, channelId,
                                             actions);
  }
}
