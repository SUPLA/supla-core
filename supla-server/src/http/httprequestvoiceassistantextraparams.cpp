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

#include "http/httprequestvoiceassistantextraparams.h"

#include <stdlib.h>
#include <string.h>

supla_http_request_voice_assistant_extra_params::
    supla_http_request_voice_assistant_extra_params(void)
    : supla_http_request_extra_params() {
  correlationToken = NULL;
  googleRequestId = NULL;
  subChannel = 0;
}

supla_http_request_voice_assistant_extra_params::
    supla_http_request_voice_assistant_extra_params(
        const char correlationToken[], const char googleRequestId[]) {
  this->correlationToken = NULL;
  this->googleRequestId = NULL;
  subChannel = 0;

  setCorrelationToken(correlationToken);
  setGoogleRequestId(googleRequestId);
}

supla_http_request_voice_assistant_extra_params::
    ~supla_http_request_voice_assistant_extra_params(void) {
  if (correlationToken) {
    free(correlationToken);
    correlationToken = NULL;
  }

  if (googleRequestId) {
    free(googleRequestId);
    googleRequestId = NULL;
  }
}

supla_http_request_extra_params*
supla_http_request_voice_assistant_extra_params::clone(void) {
  return new supla_http_request_voice_assistant_extra_params(correlationToken,
                                                             googleRequestId);
}

void supla_http_request_voice_assistant_extra_params::setCorrelationToken(
    const char correlationToken[]) {
  if (this->correlationToken) {
    free(this->correlationToken);
    this->correlationToken = NULL;
  }

  subChannel = 0;

  if (correlationToken &&
      strnlen(correlationToken, CORRELATIONTOKEN_MAXSIZE) > 0) {
    this->correlationToken =
        strndup(correlationToken, CORRELATIONTOKEN_MAXSIZE);

    char sub[] = "::SUB=";
    size_t clen = 0;

    if ((clen = strnlen(this->correlationToken, CORRELATIONTOKEN_MAXSIZE)) >=
        sizeof(sub)) {
      if (memcmp(&this->correlationToken[clen - 7], sub, sizeof(sub) - 1) ==
          0) {
        subChannel = atoi(&this->correlationToken[clen - 1]);
        this->correlationToken[clen - 7] = 0;
      }
    }
  }
}

int supla_http_request_voice_assistant_extra_params::getSubChannel(void) {
  return subChannel;
}

const char*
supla_http_request_voice_assistant_extra_params::getCorrelationTokenPtr(void) {
  return correlationToken;
}

void supla_http_request_voice_assistant_extra_params::setGoogleRequestId(
    const char googleRequestId[]) {
  if (this->googleRequestId) {
    free(this->googleRequestId);
    this->googleRequestId = NULL;
  }

  if (googleRequestId &&
      strnlen(googleRequestId, GOOGLEREQUESTID_MAXSIZE) > 0) {
    this->googleRequestId = strndup(googleRequestId, GOOGLEREQUESTID_MAXSIZE);
  }
}

const char*
supla_http_request_voice_assistant_extra_params::getGoogleRequestIdPtr(void) {
  return googleRequestId;
}
