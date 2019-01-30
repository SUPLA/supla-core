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

#include "google/googlehomesyncrequest.h"
#include "log.h"
#include "svrcfg.h"

supla_google_home_sync_request::supla_google_home_sync_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_google_home_request(user, ClassID, DeviceId, ChannelId, EventType,
                                EventSourceType) {
  setDelay(15000000);  // 15 sec.
  setTimeout(scfg_int(CFG_GOOGLE_HOME_SYNCREQUEST_TIMEOUT) * 1000);
}

bool supla_google_home_sync_request::queueUp(void) {
  return !duplicateExists && supla_google_home_request::queueUp();
}

bool supla_google_home_sync_request::verifyExisting(
    supla_http_request *existing) {
  duplicateExists = true;
  existing->setDelay(5000000);
  supla_http_request_queue::getInstance()->raiseEvent();
  return true;
};

bool supla_google_home_sync_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  return true;
}

void supla_google_home_sync_request::execute(void *sthread) {
  getClient()->requestSync();
}

bool supla_google_home_sync_request::isEventTypeAccepted(event_type eventType,
                                                         bool verification) {
  switch (eventType) {
    case ET_DEVICE_ADDED:
    case ET_DEVICE_DELETED:
    case ET_USER_RECONNECT:
      return true;
    case ET_CHANNEL_VALUE_CHANGED:
      return false;
  }
  return false;
}

REGISTER_HTTP_REQUEST_CLASS(supla_google_home_sync_request);
