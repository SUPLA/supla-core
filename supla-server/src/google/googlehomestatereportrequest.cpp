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

#include "google/googlehomestatereportrequest.h"

#include "log.h"
#include "safearray.h"
#include "svrcfg.h"
#include "user/user.h"

supla_google_home_statereport_request::supla_google_home_statereport_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, const supla_caller &Caller)
    : supla_google_home_request(user, ClassID, DeviceId, ChannelId, EventType,
                                Caller) {
  setDelay(1500000);  // 1.5 sec.
  setTimeout(scfg_int(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT) * 1000);
}

supla_google_home_statereport_request::~supla_google_home_statereport_request(
    void) {}

bool supla_google_home_statereport_request::queueUp(void) {
  return !duplicateExists && supla_google_home_request::queueUp();
}

bool supla_google_home_statereport_request::verifyExisting(
    supla_http_request *existing) {
  duplicateExists = true;

  unsigned long long delayUs = 1000000;

  if (set_google_home_request_id(existing)) {
    delayUs = 3000000;
  }

  existing->setDelay(delayUs);
  return true;
}

bool supla_google_home_statereport_request::isChannelFunctionAllowed(void) {
  channel_complex_value value =
      getUser()->get_channel_complex_value(getChannelId());

  switch (value.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_STAIRCASETIMER:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
    case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
      return !value.hidden_channel;
    default:
      return false;
  }

  return false;
}

bool supla_google_home_statereport_request::isCallerAccepted(
    const supla_caller &caller, bool verification) {
  if (!supla_google_home_request::isCallerAccepted(caller, verification)) {
    return false;
  }

  switch (caller.get_type()) {
    case ctDevice:
    case ctClient:
    case ctIPC:
    case ctMQTT:
    case ctGoogleHome:
    case ctAmazonAlexa:
    case ctScene:
    case ctActionTrigger:
    case ctCyclicTask:
      return isChannelFunctionAllowed();
    case ctUnknown:
      return false;
  }

  return false;
}

void supla_google_home_statereport_request::execute(void *sthread) {
  getClient()->clearStateReport();

  channel_complex_value value =
      getUser()->get_channel_complex_value(getChannelId());

  if (!value.hidden_channel) {
    switch (value.function) {
      case SUPLA_CHANNELFNC_POWERSWITCH:
      case SUPLA_CHANNELFNC_LIGHTSWITCH:
      case SUPLA_CHANNELFNC_STAIRCASETIMER:
        getClient()->addOnOffState(getChannelId(), value.hi, value.online);
        break;
      case SUPLA_CHANNELFNC_DIMMER:
        getClient()->addBrightnessState(getChannelId(), value.brightness,
                                        value.online, 0);
        break;
      case SUPLA_CHANNELFNC_RGBLIGHTING:
        getClient()->addColorState(getChannelId(), value.color,
                                   value.color_brightness, value.online, 0);
        break;
      case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
        getClient()->addColorState(getChannelId(), value.color,
                                   value.color_brightness, value.online, 1);
        getClient()->addBrightnessState(getChannelId(), value.brightness,
                                        value.online, 2);
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
        getClient()->addRollerShutterState(getChannelId(), value.shut,
                                           value.online);
        break;
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
      case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        getClient()->addOpenPercentState(getChannelId(), value.hi ? 0 : 100,
                                         value.online);
        break;
      default:
        return;
    }
  }

  send_report_state();
}

bool supla_google_home_statereport_request::isEventTypeAccepted(
    event_type eventType, bool verification) {
  return eventType == ET_CHANNEL_VALUE_CHANGED;
}

REGISTER_HTTP_REQUEST_CLASS(supla_google_home_statereport_request);
