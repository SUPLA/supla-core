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
  delayTime = 500000;
  duplicateExists = false;
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

bool supla_state_webhook_request::isCancelled(void *sthread) {
  if (sthread_isterminated(sthread)) {
    return true;
  }

  return !getUser()->stateWebhookCredentials()->isAccessTokenExists();
}

bool supla_state_webhook_request::verifyExisting(supla_http_request *existing) {
  duplicateExists = true;
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
              case SUPLA_CHANNELFNC_DIMMER:
              case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
              case SUPLA_CHANNELFNC_RGBLIGHTING:
                delayTime = 2500000;
                return true;
              case SUPLA_CHANNELFNC_THERMOMETER:
              case SUPLA_CHANNELFNC_HUMIDITY:
              case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE:
              case SUPLA_CHANNELFNC_WINDSENSOR:
              case SUPLA_CHANNELFNC_PRESSURESENSOR:
              case SUPLA_CHANNELFNC_RAINSENSOR:
              case SUPLA_CHANNELFNC_WEIGHTSENSOR:
              case SUPLA_CHANNELFNC_DISTANCESENSOR:
              case SUPLA_CHANNELFNC_DEPTHSENSOR:
              case SUPLA_CHANNELFNC_ELECTRICITY_METER:
              case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
              case SUPLA_CHANNELFNC_IC_GAS_METER:
              case SUPLA_CHANNELFNC_IC_WATER_METER:
              case SUPLA_CHANNELFNC_IC_HEAT_METER:
                delayTime = 15000000;
                return true;
              case SUPLA_CHANNELFNC_POWERSWITCH:
              case SUPLA_CHANNELFNC_LIGHTSWITCH:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
              case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
              case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
              case SUPLA_CHANNELFNC_MAILSENSOR:
              case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
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

void supla_state_webhook_request::electricityMeterChannelType(
    channel_complex_value *value) {
  if (value->function == SUPLA_CHANNELFNC_ELECTRICITY_METER) {
    supla_channel_electricity_measurement *em =
        getUser()->get_electricity_measurement(getDeviceId(), getChannelId());

    getClient()->sendElectricityMeasurementReport(getChannelId(), em,
                                                  value->online);

    if (em != NULL) {
      delete em;
    }
  }
}

void supla_state_webhook_request::impulseCounterChannelType(
    channel_complex_value *value) {
  supla_channel_ic_measurement *icm =
      getUser()->get_ic_measurement(getDeviceId(), getChannelId());

  switch (value->function) {
#ifdef SERVER_VERSION_23
    case SUPLA_CHANNELFNC_ELECTRICITY_METER:
#endif /*SERVER_VERSION_23*/
    case SUPLA_CHANNELFNC_IC_ELECTRICITY_METER:
      getClient()->sendImpulseCounterElectricityMeasurementReport(
          getChannelId(), icm, value->online);
      break;
    case SUPLA_CHANNELFNC_IC_GAS_METER:
      getClient()->sendImpulseCounterGasMeasurementReport(getChannelId(), icm,
                                                          value->online);
      break;
    case SUPLA_CHANNELFNC_IC_WATER_METER:
      getClient()->sendImpulseCounterWaterMeasurementReport(getChannelId(), icm,
                                                            value->online);
      break;
    case SUPLA_CHANNELFNC_IC_HEAT_METER:
      getClient()->sendImpulseCounterHeatMeasurementReport(getChannelId(), icm,
                                                           value->online);
      break;
  }

  if (icm != NULL) {
    delete icm;
  }
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
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
      getClient()->sendGatewayOpeningSensorReport(getChannelId(), value.hi,
                                                  value.online);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
      getClient()->sendGateOpeningSensorReport(getChannelId(), value.hi,
                                               value.online);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
      getClient()->sendGarageDoorOpeningSensorReport(getChannelId(), value.hi,
                                                     value.online);
      break;
    case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
      getClient()->sendNoLiquidSensorReport(getChannelId(), value.hi,
                                            value.online);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
      getClient()->sendDoorOpeningSensorReport(getChannelId(), value.hi,
                                               value.online);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
      getClient()->sendRollerShutterOpeningSensorReport(getChannelId(),
                                                        value.hi, value.online);
      break;
    case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:
      getClient()->sendWindowOpeningSensorReport(getChannelId(), value.hi,
                                                 value.online);
      break;
    case SUPLA_CHANNELFNC_MAILSENSOR:
      getClient()->sendMailSensorReport(getChannelId(), value.hi, value.online);
      break;
    case SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER:
      getClient()->sendRollerShutterReport(getChannelId(), value.shut,
                                           value.online);
      break;
    case SUPLA_CHANNELFNC_WINDSENSOR:
      getClient()->sendWindSensorReport(getChannelId(), value.wind,
                                        value.online);
      break;
    case SUPLA_CHANNELFNC_PRESSURESENSOR:
      getClient()->sendPressureSensorReport(getChannelId(), value.pressure,
                                            value.online);
      break;
    case SUPLA_CHANNELFNC_RAINSENSOR:
      getClient()->sendRainSensorReport(getChannelId(), value.rain,
                                        value.online);
      break;
    case SUPLA_CHANNELFNC_WEIGHTSENSOR:
      getClient()->sendWeightSensorReport(getChannelId(), value.weight,
                                          value.online);
      break;
    case SUPLA_CHANNELFNC_DISTANCESENSOR:
      getClient()->sendDistanceSensorReport(getChannelId(), value.distance,
                                            value.online);
      break;
    case SUPLA_CHANNELFNC_DEPTHSENSOR:
      getClient()->sendDepthSensorReport(getChannelId(), value.depth,
                                         value.online);
      break;
    case SUPLA_CHANNELFNC_DIMMER:

      getClient()->sendDimmerReport(getChannelId(), value.brightness,
                                    value.on_off, value.online);

      break;
    case SUPLA_CHANNELFNC_DIMMERANDRGBLIGHTING:
      getClient()->sendDimmerAndRgbReport(
          getChannelId(), value.color, value.color_brightness, value.brightness,
          value.on_off, value.online);
      break;
    case SUPLA_CHANNELFNC_RGBLIGHTING:
      getClient()->sendRgbReport(getChannelId(), value.color,
                                 value.color_brightness, value.on_off,
                                 value.online);
      break;
  }

  switch (value.channel_type) {
    case SUPLA_CHANNELTYPE_ELECTRICITY_METER:
      electricityMeterChannelType(&value);
      break;
    case SUPLA_CHANNELTYPE_IMPULSE_COUNTER:
      impulseCounterChannelType(&value);
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
  setDelay(delayTime);
}

REGISTER_HTTP_REQUEST_CLASS(supla_state_webhook_request);
