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

#include "google/googlehomerequest.h"
#include <assert.h>
#include "lck.h"
#include "sthread.h"
#include "user/user.h"

supla_google_home_request::supla_google_home_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_http_request(user, ClassID, DeviceId, ChannelId, EventType,
                         EventSourceType) {
  client = NULL;
  lck = lck_init();
  duplicateExists = false;
}

supla_google_home_request::~supla_google_home_request() {
  lck_lock(lck);
  if (client) {
    delete client;
    client = NULL;
  }
  lck_unlock(lck);

  lck_free(lck);
}

bool supla_google_home_request::isCancelled(void *sthread) {
  if (sthread_isterminated(sthread)) {
    return true;
  }

  return !getUser()->googleHome()->isAccessTokenExists();
}

void supla_google_home_request::terminate(void *sthread) {
  lck_lock(lck);
  if (client) {
    client->terminate();
  }
  lck_unlock(lck);
  supla_http_request::terminate(sthread);
}

bool supla_google_home_request::queueUp(void) { return true; }

supla_google_home_client *supla_google_home_request::getClient(void) {
  supla_google_home *google_home = getUser()->googleHome();
  assert(google_home != NULL);

  supla_google_home_client *result = NULL;
  lck_lock(lck);
  if (!client) {
    client = new supla_google_home_client(google_home);
  }
  result = client;
  lck_unlock(lck);

  return result;
}

bool supla_google_home_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  supla_google_home *google_home = getUser()->googleHome();
  return google_home && google_home->isAccessTokenExists();
}

bool supla_google_home_request::isDeviceIdEqual(int DeviceId) { return true; }

bool supla_google_home_request::isChannelIdEqual(int ChannelId) { return true; }
