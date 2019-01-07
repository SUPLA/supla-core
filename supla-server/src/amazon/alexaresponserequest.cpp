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
#include <unistd.h>
#include "http/httprequestqueue.h"
#include "log.h"
#include "sthread.h"
#include "svrcfg.h"
#include "user/user.h"

supla_alexa_response_request::supla_alexa_response_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_alexa_request(user, ClassID, DeviceId, ChannelId, EventType,
                          EventSourceType) {
  setDelay(1000000);
  setTimeout(scfg_int(CFG_ALEXA_RESPONSE_TIMEOUT) * 1000);
}

supla_alexa_response_request::~supla_alexa_response_request() {}

bool supla_alexa_response_request::verifyExisting(
    supla_http_request *existing) {
  if (getEventSourceType() == EST_DEVICE) {
    existing->setDelay(0);
    supla_http_request_queue::getInstance()->raiseEvent();
    return true;
  }
  return false;
}

bool supla_alexa_response_request::queueUp(void) {
  return getEventSourceType() == EST_AMAZON_ALEXA &&
         supla_alexa_request::queueUp();
}

bool supla_alexa_response_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  if (!supla_alexa_request::isEventSourceTypeAccepted(eventSourceType,
                                                      verification)) {
    return false;
  }

  channel_complex_value summary =
      getUser()->get_channel_complex_value(getDeviceId(), getChannelId());

  switch (summary.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      if (verification) {
        if (eventSourceType == EST_DEVICE ||
            eventSourceType == EST_AMAZON_ALEXA) {
          return true;
        }
      } else if (eventSourceType == EST_AMAZON_ALEXA) {
        return true;
      }
      break;
    default:
      return false;
  }

  return false;
}

void supla_alexa_response_request::execute(void *sthread) {
  channel_complex_value value =
      getUser()->get_channel_complex_value(getDeviceId(), getChannelId());

  switch (value.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      getClient()->powerControllerSendResponse(
          getCorrelationTokenPtr(), getChannelId(), value.hi, value.online);
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      getClient()->brightnessControllerSendResponse(
          getCorrelationTokenPtr(), getChannelId(), value.brightness,
          value.online, 0);
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      getClient()->colorControllerSendResponse(
          getCorrelationTokenPtr(), getChannelId(), value.color,
          value.color_brightness, value.online, 0);
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      if (getSubChannelFromCorrelationToken() == 1) {
        getClient()->colorControllerSendResponse(
            getCorrelationTokenPtr(), getChannelId(), value.color,
            value.color_brightness, value.online, 1);
      } else if (getSubChannelFromCorrelationToken() == 2) {
        getClient()->brightnessControllerSendResponse(
            getCorrelationTokenPtr(), getChannelId(), value.brightness,
            value.online, 2);
      }
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      getClient()->percentageControllerSendResponse(
          getCorrelationTokenPtr(), getChannelId(), value.shut, value.online);
      break;
  }
}

REGISTER_HTTP_REQUEST_CLASS(supla_alexa_response_request);
