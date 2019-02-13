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

#include "voiceassistantclient.h"
#include <stdlib.h>
#include <string.h>
#include "http/trivialhttps.h"
#include "lck.h"
#include "user/user.h"
#include "voiceassistant.h"

supla_voice_assistant_client::supla_voice_assistant_client(
    supla_voice_assistant *voice_assistant) {
  this->lck = lck_init();
  this->https = NULL;
  this->voice_assistant = voice_assistant;
}

void supla_voice_assistant_client::httpsInit(void) {
  httpsFree();
  lck_lock(lck);
  https = new supla_trivial_https();
  lck_unlock(lck);
}

void supla_voice_assistant_client::httpsFree(void) {
  lck_lock(lck);
  if (https) {
    delete https;
    https = NULL;
  }
  lck_unlock(lck);
}

void supla_voice_assistant_client::terminate(void) {
  lck_lock(lck);
  if (https) {
    https->terminate();
  }
  lck_unlock(lck);
}

supla_trivial_https *supla_voice_assistant_client::getHttps(void) {
  supla_trivial_https *result = NULL;
  lck_lock(lck);
  if (!https) {
    httpsInit();
  }
  result = https;
  lck_unlock(lck);
  return result;
}

supla_voice_assistant *supla_voice_assistant_client::getVoiceAssistant(void) {
  return voice_assistant;
}

supla_voice_assistant_client::~supla_voice_assistant_client() {
  httpsFree();
  lck_free(lck);
}

char *supla_voice_assistant_client::getEndpointId(int channelId,
                                                  short subChannel) {
  char *result = NULL;
  char *uuid = getVoiceAssistant()->getUser()->getShortUniqueID();
  if (!uuid) {
    return NULL;
  }

  int endpointId_len = (uuid ? strnlen(uuid, SHORT_UNIQUEID_MAXSIZE) : 0) + 30;
  char *endpointId = (char *)malloc(endpointId_len + 1);

  if (endpointId) {
    if (subChannel) {
      snprintf(endpointId, endpointId_len, "%s-%i-%i", uuid ? uuid : "",
               channelId, subChannel);
    } else {
      snprintf(endpointId, endpointId_len, "%s-%i", uuid ? uuid : "",
               channelId);
    }

    result = endpointId;
  }

  free(uuid);
  return result;
}
