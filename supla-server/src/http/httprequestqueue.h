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

#ifndef HTTP_HTTPREQUESTQUEUE_H_
#define HTTP_HTTPREQUESTQUEUE_H_

#include <string>

#include "caller.h"
#include "user/user.h"

class supla_http_request_queue {
 public:
  static void onChannelValueChangeEvent(supla_user *user, int deviceId,
                                        int channelId,
                                        const supla_caller &caller,
                                        const char correlationToken[] = nullptr,
                                        const char googleRequestId[] = nullptr);

  static void onChannelsAddedEvent(supla_user *user, int deviceId,
                                   const supla_caller &caller);

  static void onDeviceDeletedEvent(supla_user *user, int deviceId,
                                   const supla_caller &caller);

  static void onUserReconnectEvent(supla_user *user,
                                   const supla_caller &caller);

  static void onGoogleHomeSyncNeededEvent(supla_user *user,
                                          const supla_caller &caller);

  static void onActionsTriggered(const supla_caller &caller, supla_user *user,
                                 int deviceId, int channelId,
                                 unsigned int actions);
};

#endif /* HTTP_HTTPREQUESTQUEUE_H_ */
