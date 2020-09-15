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

#include "webhook/statewebhookrequest.h"
#include <assert.h>
#include "lck.h"
#include "sthread.h"
#include "user/user.h"
#include "webhook/statewebhookcredentials.h"

supla_state_webhook_request::supla_state_webhook_request(
    supla_user *user, int ClassID, int DeviceId, int ChannelId,
    event_type EventType, event_source_type EventSourceType)
    : supla_http_request(user, ClassID, DeviceId, ChannelId, EventType,
                         EventSourceType) {
  client = NULL;
  duplicateExists = false;
  measuringSensor = false;
  lck = lck_init();
}

supla_state_webhook_request::~supla_state_webhook_request() {
  lck_lock(lck);
  if (client) {
    delete client;
    client = NULL;
  }
  lck_unlock(lck);

  lck_free(lck);
}

bool supla_state_webhook_request::isMeasuringSensor(void) {
  return measuringSensor;
}

bool supla_state_webhook_request::isCancelled(void *sthread) {
  if (sthread_isterminated(sthread)) {
    return true;
  }

  return !getUser()->stateWebhookCredentials()->isAccessTokenExists();
}

bool supla_state_webhook_request::verifyExisting(supla_http_request *existing) {
  duplicateExists = true;
  if (!static_cast<supla_state_webhook_request *>(existing)
           ->isMeasuringSensor()) {
    supla_http_request_queue::getInstance()->raiseEvent();
  }

  return true;
}

bool supla_state_webhook_request::queueUp(void) { return !duplicateExists; }

bool supla_state_webhook_request::isEventSourceTypeAccepted(
    event_source_type eventSourceType, bool verification) {
  supla_state_webhook_credentials *credentials =
      getUser()->stateWebhookCredentials();
  if (credentials == NULL || !credentials->isAccessTokenExists() ||
      !credentials->isUrlValid()) {
    return false;
  }

  switch (eventSourceType) {
    case EST_DEVICE:
    case EST_CLIENT:
    case EST_AMAZON_ALEXA:
    case EST_GOOGLE_HOME:
    case EST_IPC: {
      channel_complex_value value =
          getUser()->get_channel_complex_value(getChannelId());

      if (!value.hidden_channel) {
        std::list<int> fids = credentials->getFunctionsIds();
        for (int f : fids) {
          if (f == value.function) {
            switch (value.function) {
              case SUPLA_CHANNELFNC_THERMOMETER:
              case SUPLA_CHANNELFNC_HUMIDITY:
              case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
                measuringSensor = true;
                return true;
              case SUPLA_CHANNELFNC_POWERSWITCH:
              case SUPLA_CHANNELFNC_LIGHTSWITCH:
                return true;
              default:
                return false;
            }
          }
        }
      }

      return false;
    } break;
    case EST_UNKNOWN:
      return false;
  }

  return false;
}

bool supla_state_webhook_request::isEventTypeAccepted(event_type eventType,
                                                      bool verification) {
  return eventType == ET_CHANNEL_VALUE_CHANGED;
}

supla_state_webhook_client *supla_state_webhook_request::getClient(void) {
  supla_state_webhook_credentials *credentials =
      getUser()->stateWebhookCredentials();
  assert(credentials != NULL);

  supla_state_webhook_client *result = NULL;
  lck_lock(lck);
  if (!client) {
    client = new supla_state_webhook_client(credentials);
  }
  result = client;
  lck_unlock(lck);

  return result;
}

void supla_state_webhook_request::execute(void *sthread) {
  channel_complex_value value =
      getUser()->get_channel_complex_value(getChannelId());

  switch (value.function) {
    case SUPLA_CHANNELFNC_POWERSWITCH:
      getClient()->sendPowerSwitchReport(getChannelId(), value.hi,
                                         value.online);
      break;
    case SUPLA_CHANNELFNC_LIGHTSWITCH:
      getClient()->sendLightSwitchReport(getChannelId(), value.hi,
                                         value.online);
      break;

    case SUPLA_CHANNELFNC_THERMOMETER:
      getClient()->sendTemperatureReport(getChannelId(), value.temperature,
                                         value.online);
      break;
    case SUPLA_CHANNELFNC_HUMIDITY:
      getClient()->sendHumidityReport(getChannelId(), value.humidity,
                                      value.online);
      break;
    case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
      getClient()->sendTemperatureAndHumidityReport(
          getChannelId(), value.temperature, value.humidity, value.online);
      break;
  }
}

void supla_state_webhook_request::terminate(void *sthread) {
  lck_lock(lck);
  if (client) {
    client->terminate();
  }
  lck_unlock(lck);
  supla_http_request::terminate(sthread);
}

void supla_state_webhook_request::requestWillBeAdded(void) {
  setDelay(measuringSensor ? 30000000 : 500000);
}

REGISTER_HTTP_REQUEST_CLASS(supla_state_webhook_request);
