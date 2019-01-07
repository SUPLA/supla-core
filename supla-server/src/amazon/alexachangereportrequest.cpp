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

#include <amazon/alexachangereportrequest.h>
#include "log.h"
#include "sthread.h"
#include "svrcfg.h"
#include "user/user.h"

supla_alexa_changereport_request::supla_alexa_changereport_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_alexa_request(user, ClassID, DeviceId, ChannelId, EventType,
                          EventSourceType) {
  duplicateExists = false;
  setDelay(1500000);  // 1.5 sec.
  setTimeout(scfg_int(CFG_ALEXA_CHANGEREPORT_TIMEOUT) * 1000);
}

supla_alexa_changereport_request::~supla_alexa_changereport_request() {}

bool supla_alexa_changereport_request::queueUp(void) {
  return !duplicateExists && supla_alexa_request::queueUp();
}

bool supla_alexa_changereport_request::verifyExisting(
    supla_http_request *existing) {
  duplicateExists = true;
  existing->setDelay(1000000);
  supla_http_request_queue::getInstance()->raiseEvent();
  return true;
};

bool supla_alexa_changereport_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  if (!supla_alexa_request::isEventSourceTypeAccepted(eventSourceType,
                                                      verification)) {
    return false;
  }

  switch (eventSourceType) {
    case EST_DEVICE:
    case EST_CLIENT:
    case EST_AMAZON_ALEXA:
    case EST_IPC: {
      channel_complex_value value =
          getUser()->get_channel_complex_value(getDeviceId(), getChannelId());

      switch (value.function) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_DIMMER:
        case SUPLA_CHANNELFNC_RGBLIGHTING:
        case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
          return true;
        default:
          return false;
      }
    } break;
    case EST_UNKNOWN:
    case EST_GOOGLE_HOME:
      return false;
  };

  return false;
}

void supla_alexa_changereport_request::execute(void *sthread) {
  channel_complex_value value =
      getUser()->get_channel_complex_value(getDeviceId(), getChannelId());

  switch (value.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      getClient()->sendPowerChangeReport(getCauseType(), getChannelId(),
                                         value.hi, value.online);
      break;
    case SUPLA_CHANNELFNC_DIMMER:
      getClient()->sendBrightnessChangeReport(
          getCauseType(), getChannelId(), value.brightness, value.online, 0);
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      getClient()->sendColorChangeReport(getCauseType(), getChannelId(),
                                         value.color, value.color_brightness,
                                         value.online, 0);
      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      getClient()->sendColorChangeReport(getCauseType(), getChannelId(),
                                         value.color, value.color_brightness,
                                         value.online, 1);
      getClient()->sendBrightnessChangeReport(
          getCauseType(), getChannelId(), value.brightness, value.online, 2);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      getClient()->sendContactChangeReport(getCauseType(), getChannelId(),
                                           value.hi, value.online);
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      getClient()->sendPercentageChangeReport(getCauseType(), getChannelId(),
                                              value.shut, value.online);
      break;
  }
}

REGISTER_HTTP_REQUEST_CLASS(supla_alexa_changereport_request);
