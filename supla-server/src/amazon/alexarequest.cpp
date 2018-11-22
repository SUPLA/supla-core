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
#include <stdlib.h>
#include "amazon/alexa.h"
#include "amazon/alexaclient.h"
#include "lck.h"
#include "user/user.h"

supla_alexa_request::supla_alexa_request(supla_user *user, int ClassID,
                                         int DeviceId, int ChannelId,
                                         event_source_type EventSourceType)
    : supla_http_request(user, ClassID, DeviceId, ChannelId, EventSourceType) {
  client = NULL;
  lck = lck_init();
  subChannelFromCorrelationToken = 0;
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
  supla_amazon_alexa *alexa = getUser()->amazonAlexa();
  if (!alexa || !alexa->isAccessTokenExists()) {
    return NULL;
  }

  supla_alexa_client *result = NULL;
  lck_lock(lck);
  if (!client) {
    client = new supla_alexa_client(alexa);
  }
  result = client;
  lck_unlock(lck);

  return result;
}

bool supla_alexa_request::isEventSourceTypeAccepted(short eventSourceType,
                                                    bool verification) {
  supla_amazon_alexa *alexa = getUser()->amazonAlexa();
  return alexa && alexa->isAccessTokenExists() &&
         getUser()->is_device_online(getDeviceId());
}

int supla_alexa_request::getCauseType(void) {
  switch (getEventSourceType()) {
    case EST_AMAZON_ALEXA:
      return CAUSE_VOICE_INTERACTION;

    case EST_CLIENT:
    case EST_IPC:
    case EST_GOOGLE_HOME:
      return CAUSE_APP_INTERACTION;

    default:
      return CAUSE_PHYSICAL_INTERACTION;
  }
}

int supla_alexa_request::getSubChannelFromCorrelationToken(void) {
  return subChannelFromCorrelationToken;
}

void supla_alexa_request::setCorrelationToken(const char correlationToken[]) {
  supla_http_request::setCorrelationToken(correlationToken);
  subChannelFromCorrelationToken = 0;

  char sub[] = "::SUB=";
  size_t clen = 0;

  if (this->correlationToken &&
      (clen = strnlen(this->correlationToken, CORRELATIONTOKEN_MAXSIZE)) >=
          sizeof(sub)) {
    if (memcmp(&this->correlationToken[clen - 7], sub, sizeof(sub) - 1) == 0) {
      subChannelFromCorrelationToken = atoi(&this->correlationToken[clen - 1]);
      this->correlationToken[clen - 7] = 0;
    }
  }
}
