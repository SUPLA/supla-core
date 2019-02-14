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
    event_type EventType, event_source_type EventSourceType)
    : supla_google_home_request(user, ClassID, DeviceId, ChannelId, EventType,
                                EventSourceType) {
  channel_arr = safe_array_init();
  addChannelId(ChannelId);
  setDelay(1500000);  // 1.5 sec.
  setTimeout(scfg_int(CFG_GOOGLE_HOME_STATEREPORT_TIMEOUT) * 1000);
}

supla_google_home_statereport_request::~supla_google_home_statereport_request(
    void) {
  safe_array_free(channel_arr);
}

void supla_google_home_statereport_request::addChannelId(int ChannelId) {
  safe_array_lock(channel_arr);
  bool exists = false;

  for (int a = 0; a < safe_array_count(channel_arr); a++) {
    if ((long long)safe_array_get(channel_arr, a) == ChannelId) {
      exists = true;
      break;
    }
  }

  if (!exists) {
    safe_array_add(channel_arr, (void *)(long long)ChannelId);
  }

  safe_array_unlock(channel_arr);
}

bool supla_google_home_statereport_request::queueUp(void) {
  return !duplicateExists && supla_google_home_request::queueUp();
}

bool supla_google_home_statereport_request::verifyExisting(
    supla_http_request *existing) {
  duplicateExists = true;
  static_cast<supla_google_home_statereport_request *>(existing)->addChannelId(
      getChannelId());

  if (getGoogleRequestIdPtr() != NULL) {
    existing->setGoogleRequestId(getGoogleRequestIdPtr());
  }

  existing->setDelay(existing->getGoogleRequestIdPtr() ? 3000000 : 1000000);

  supla_http_request_queue::getInstance()->raiseEvent();
  return true;
};

bool supla_google_home_statereport_request::isChannelFunctionAllowed(void) {
  channel_complex_value value =
      getUser()->get_channel_complex_value(getDeviceId(), getChannelId());

  switch (value.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
    case SUPLA_CHANNELFNC_DIMMER:
    case SUPLA_CHANNELFNC_RGBLIGHTING:
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      return !value.hidden_channel;
    default:
      return false;
  }

  return false;
}

bool supla_google_home_statereport_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  if (!supla_google_home_request::isEventSourceTypeAccepted(eventSourceType,
                                                            verification)) {
    return false;
  }

  switch (eventSourceType) {
    case EST_DEVICE:
    case EST_CLIENT:
    case EST_IPC:
    case EST_GOOGLE_HOME:
    case EST_AMAZON_ALEXA:
      return isChannelFunctionAllowed();
    case EST_UNKNOWN:
      return false;
  }

  return false;
}

void supla_google_home_statereport_request::execute(void *sthread) {
  safe_array_lock(channel_arr);

  getClient()->clearStateReport();
  int content_exists = false;

  for (int a = 0; a < safe_array_count(channel_arr); a++) {
    int ChannelId = (long long)safe_array_get(channel_arr, a);

    channel_complex_value value =
        getUser()->get_channel_complex_value(getDeviceId(), ChannelId);

    if (!value.hidden_channel) {
      switch (value.function) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
          getClient()->addOnOffState(ChannelId, value.hi, value.online);
          content_exists = true;
          break;
        case SUPLA_CHANNELFNC_DIMMER:
          getClient()->addBrightnessState(ChannelId, value.brightness,
                                          value.online, 0);
          content_exists = true;
          break;
        case SUPLA_CHANNELFNC_RGBLIGHTING:
          getClient()->addColorState(ChannelId, value.color,
                                     value.color_brightness, value.online, 0);
          content_exists = true;
          break;
        case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
          getClient()->addColorState(ChannelId, value.color,
                                     value.color_brightness, value.online, 1);
          getClient()->addBrightnessState(ChannelId, value.brightness,
                                          value.online, 2);
          content_exists = true;
          break;
      }
    }
  }

  safe_array_unlock(channel_arr);

  if (content_exists) {
    int resultCode = 0;
    getClient()->sendReportState(getGoogleRequestIdPtr(), &resultCode);

    if (resultCode == 404) {
      getUser()->googleHome()->on_reportstate_404_error();
    }
  }
}

bool supla_google_home_statereport_request::isEventTypeAccepted(
    event_type eventType, bool verification) {
  return eventType == ET_CHANNEL_VALUE_CHANGED;
}

REGISTER_HTTP_REQUEST_CLASS(supla_google_home_statereport_request);
