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

#ifndef HTTP_EVENT_HUB_H_
#define HTTP_EVENT_HUB_H_

#include <string>

#include "amazon/alexa_correlation_token.h"
#include "caller.h"
#include "user/user.h"

class supla_http_event_hub {
 public:
  static void on_channel_value_change(supla_user *user, int deviceId,
                                      int channelId,
                                      const supla_caller &caller);

  static void on_value_change_request(
      supla_user *user, int deviceId, int channelId, const supla_caller &caller,
      supla_alexa_correlation_token *correlationToken,
      const char googleRequestId[]);

  static void on_channel_added(supla_user *user, int deviceId,
                               const supla_caller &caller);

  static void before_device_delete(supla_user *user, int deviceId,
                                   const supla_caller &caller);

  static void on_device_deleted(supla_user *user, int deviceId,
                                const supla_caller &caller);

  static void on_user_reconnect(supla_user *user, const supla_caller &caller);

  static void on_voice_assistant_sync_needed(supla_user *user,
                                             const supla_caller &caller);

  static void on_actions_triggered(const supla_caller &caller, supla_user *user,
                                   int deviceId, int channelId,
                                   unsigned int actions);

  static void on_scene_removed(supla_user *user, int sceneId,
                               const supla_caller &caller);
};

#endif /* HTTP_EVENT_HUB_H_ */
