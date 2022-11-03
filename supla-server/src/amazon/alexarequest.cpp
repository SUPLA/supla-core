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

#include "amazon/alexarequest.h"

#include <amazon/alexacredentials.h>
#include <assert.h>
#include <stdlib.h>

#include "amazon/alexaclient.h"
#include "lck.h"
#include "log.h"
#include "sthread.h"
#include "user/user.h"

supla_alexa_request::supla_alexa_request(supla_user *user, int ClassID,
                                         int DeviceId, int ChannelId,
                                         event_type EventType,
                                         const supla_caller &Caller)
    : supla_http_request(user, ClassID, DeviceId, ChannelId, EventType,
                         Caller) {
  client = NULL;
  lck = lck_init();
}

supla_alexa_request::~supla_alexa_request() {
  lck_lock(lck);
  if (client) {
    delete client;
    client = NULL;
  }
  lck_unlock(lck);

  lck_free(lck);
}

bool supla_alexa_request::isCancelled(void *sthread) {
  if (sthread_isterminated(sthread)) {
    return true;
  }

  return !getUser()->amazonAlexaCredentials()->isAccessTokenExists();
}

void supla_alexa_request::terminate(void *sthread) {
  lck_lock(lck);
  if (client) {
    client->terminate();
  }
  lck_unlock(lck);
  supla_http_request::terminate(sthread);
}

bool supla_alexa_request::queueUp(void) { return true; }

supla_alexa_client *supla_alexa_request::getClient(void) {
  supla_amazon_alexa_credentials *alexa = getUser()->amazonAlexaCredentials();
  assert(alexa != NULL);

  supla_alexa_client *result = NULL;
  lck_lock(lck);
  if (!client) {
    client = new supla_alexa_client(alexa);
  }
  result = client;
  lck_unlock(lck);

  return result;
}

bool supla_alexa_request::isCallerAccepted(const supla_caller &caller,
                                           bool verification) {
  supla_amazon_alexa_credentials *alexa = getUser()->amazonAlexaCredentials();

  return alexa && alexa->isAccessTokenExists() &&
         getUser()->get_devices()->is_online(getDeviceId());
}

bool supla_alexa_request::isEventTypeAccepted(event_type eventType,
                                              bool verification) {
  return eventType == ET_CHANNEL_VALUE_CHANGED;
}

int supla_alexa_request::getCauseType(void) {
  switch (getCaller().get_type()) {
    case ctAmazonAlexa:
      return CAUSE_VOICE_INTERACTION;

    case ctClient:
    case ctIPC:
    case ctMQTT:
    case ctGoogleHome:
    case ctScene:
      return CAUSE_APP_INTERACTION;

    default:
      return CAUSE_PHYSICAL_INTERACTION;
  }
}
