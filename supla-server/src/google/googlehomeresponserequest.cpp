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

#include "google/googlehomeresponserequest.h"
#include "log.h"

supla_google_home_response_request::supla_google_home_response_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_google_home_statereport_request(user, ClassID, DeviceId, ChannelId,
                                            EventType, EventSourceType){};

bool supla_google_home_response_request::verifyExisting(
    supla_http_request *existing) {
  if (getEventSourceType() == EST_DEVICE) {
    existing->setDelay(0);
    supla_http_request_queue::getInstance()->raiseEvent();
    return true;
  }
  return false;
}

bool supla_google_home_response_request::queueUp(void) {
  return getEventSourceType() == EST_GOOGLE_HOME &&
         supla_google_home_request::queueUp();
}

bool supla_google_home_response_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  if (!supla_google_home_request::isEventSourceTypeAccepted(eventSourceType,
                                                            verification)) {
    return false;
  }

  if (isChannelFunctionAllowed()) {
    if (verification) {
      if (eventSourceType == EST_DEVICE || eventSourceType == EST_GOOGLE_HOME) {
        return true;
      }
    } else if (eventSourceType == EST_GOOGLE_HOME) {
      return true;
    }
  }

  return false;
}

REGISTER_HTTP_REQUEST_CLASS(supla_google_home_response_request);
