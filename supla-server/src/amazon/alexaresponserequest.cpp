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

#include "amazon/alexaresponserequest.h"
#include "http/httprequestqueue.h"
#include "log.h"

supla_alexa_response_request::supla_alexa_response_request(supla_user *user,
                                                           int ClassID)
    : supla_http_request(user, ClassID) {}

supla_alexa_response_request::~supla_alexa_response_request() {}

bool supla_alexa_response_request::verifyExisting(
    supla_http_request *existing) {
  return false;
}

bool supla_alexa_response_request::queueUp(void) { return true; }

bool supla_alexa_response_request::isEventSourceTypeAccepted(
    short eventSourceType, bool verification) {
  if (verification) {
    switch (eventSourceType) {
      case HTTP_EVENT_SOURCE_DEVICE:
      case HTTP_EVENT_SOURCE_AMAZON_ALEXA:
        return true;
    }
  } else if (eventSourceType == HTTP_EVENT_SOURCE_AMAZON_ALEXA) {
    return true;
  }

  return false;
}

REGISTER_HTTP_REQUEST_CLASS(supla_alexa_response_request);
