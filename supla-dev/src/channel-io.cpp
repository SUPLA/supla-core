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

#include "channel-io.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gpio.h"
#include "mcp23008.h"
#include "supla-client-lib/eh.h"
#include "supla-client-lib/lck.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/safearray.h"
#include "supla-client-lib/sthread.h"
#include "w1.h"

// TODO(pzygmunt) whole remodel

#define GPIO_MINDELAY_USEC 500000
#ifdef __SINGLE_THREAD
#define W1_TEMP_MINDELAY_SEC 120
#else
#define W1_TEMP_MINDELAY_USEC 500000
#endif
#define MCP23008_MINDELAY_SEC 1

void (*mqtt_publish_callback)(const char *topic, const char *payload,
                              char retain, char qos);

client_device_channels *channels = NULL;

#ifndef __SINGLE_THREAD
void channelio_w1_execute(void *user_data, void *sthread);
void channelio_gpio_monitor_execute(void *user_data, void *sthread);
void channelio_mcp23008_execute(void *user_data, void *sthread);
#endif

void channelio_raise_valuechanged(client_device_channel *channel) {
  char value[SUPLA_CHANNELVALUE_SIZE];

  channel->getValue(value);

  if (channels->on_valuechanged)
    channels->on_valuechanged(channel->getNumber(), value,
                              channels->on_valuechanged_user_data);
}

char channelio_read_from_file(client_device_channel *channel, char log_err) {
  double val1 = -275, val2 = -1;
  struct timeval now;
  char read_result = 0;

  if (channel->getFileName().length() > 0) {
    gettimeofday(&now, NULL);

    int interval_sec = channel->getIntervalSec();
    int min_interval = interval_sec >= 0 ? interval_sec : 10;

    if (now.tv_sec - channel->getLastSeconds() >= min_interval) {
      channel->setLastSeconds();

      read_result =
          file_read_sensor(channel->getFileName().c_str(), &val1, &val2);

      char tmp_value[SUPLA_CHANNELVALUE_SIZE];

      switch (channel->getFunction()) {
        case SUPLA_CHANNELFNC_POWERSWITCH:
        case SUPLA_CHANNELFNC_LIGHTSWITCH:
        case SUPLA_CHANNELFNC_STAIRCASETIMER:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEDOORLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGARAGEDOOR:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATEWAYLOCK:
        case SUPLA_CHANNELFNC_CONTROLLINGTHEGATE:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATEWAY:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GATE:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_GARAGEDOOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_DOOR:
        case SUPLA_CHANNELFNC_NOLIQUIDSENSOR:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_ROLLERSHUTTER:
        case SUPLA_CHANNELFNC_OPENINGSENSOR_WINDOW:  // ver. >= 8
        case SUPLA_CHANNELFNC_MAILSENSOR: {
          tmp_value[0] = val1 == 1 ? 1 : 0;
        } break;
        case SUPLA_CHANNELFNC_THERMOMETER:
        case SUPLA_CHANNELFNC_DISTANCESENSOR:
        case SUPLA_CHANNELFNC_DEPTHSENSOR:
        case SUPLA_CHANNELFNC_WINDSENSOR:
        case SUPLA_CHANNELFNC_PRESSURESENSOR:
        case SUPLA_CHANNELFNC_RAINSENSOR:
        case SUPLA_CHANNELFNC_WEIGHTSENSOR: {
          memcpy(tmp_value, &val1, sizeof(double));
        } break;
        case SUPLA_CHANNELFNC_HUMIDITYANDTEMPERATURE: {
          int n;

          n = val1 * 1000;
          memcpy(tmp_value, &n, 4);

          n = val2 * 1000;
          memcpy(&tmp_value[4], &n, 4);
        } break;
      }

      channel->setValue(tmp_value);

      if (read_result == 0 && log_err == 1)
        supla_log(LOG_ERR, "Can't read file %s",
                  channel->getFileName().c_str());
    }
  }
  return read_result;
}

char channelio_gpio_in(TDeviceChannel *channel, char port12) {
  if (channel->type == SUPLA_CHANNELTYPE_SENSORNO ||
      channel->type == SUPLA_CHANNELTYPE_SENSORNC ||
      ((channel->type == SUPLA_CHANNELTYPE_RELAYHFD4 ||
        channel->type == SUPLA_CHANNELTYPE_RELAYG5LA1A ||
        channel->type == SUPLA_CHANNELTYPE_RELAYG5LA1A) &&
       port12 == 2 && channel->bistable == 1))
    return 1;

  return 0;
}

void mqtt_subscribe_callback(void **state,
                             struct mqtt_response_publish *publish) {
  std::string topic =
      std::string((char *)publish->topic_name, publish->topic_name_size);
  std::string message = std::string((char *)publish->application_message,
                                    publish->application_message_size);
  client_device_channel *channel =
      channels->find_channel_by_topic(topic.c_str());

  if (channel) {
    handle_subscribed_message(channel, topic, message,
                              channels->on_valuechanged,
                              channels->on_valuechanged_user_data);

    channelio_raise_execute_command(channel);
  }
}

char channelio_init(void) {
  channels = new client_device_channels();
  return 1;
}

void channelio_free(void) {
  if (channels) {
    delete channels;
  }
}

char channelio_allowed_type(int type) {
  switch (type) {
    case SUPLA_CHANNELTYPE_RELAYHFD4:
    case SUPLA_CHANNELTYPE_SENSORNO:
    case SUPLA_CHANNELTYPE_SENSORNC:
    case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
    case SUPLA_CHANNELTYPE_RELAYG5LA1A:
    case SUPLA_CHANNELTYPE_2XRELAYG5LA1A:
    case SUPLA_CHANNELTYPE_DHT11:
    case SUPLA_CHANNELTYPE_DHT22:
    case SUPLA_CHANNELTYPE_AM2302:
    case SUPLA_CHANNELTYPE_DIMMER:
    case SUPLA_CHANNELTYPE_RGBLEDCONTROLLER:
    case SUPLA_CHANNELTYPE_DIMMERANDRGBLED:
    case SUPLA_CHANNELTYPE_HUMIDITYSENSOR:
    case SUPLA_CHANNELTYPE_RELAY:
      return 1;
  }

  return 0;
}

void channelio_channel_init(void) {
  if (channels->getInitialized()) return;

  int a;

  client_device_channel *channel;
  for (a = 0; a < channels->getChannelCount(); a++) {
    channel = channels->getChannel(a);

    if (channel->getFileName().length() == 0) continue;

    if (channelio_read_from_file(channel, 1)) {
      channelio_raise_valuechanged(channel);
    };
  }

  channels->setInitialized(true);
#ifndef __SINGLE_THREAD
  sthread_simple_run(channelio_w1_execute, NULL, 0);
#endif
}

int channelio_channel_count(void) { return channels->getChannelCount(); }

void channelio_set_payload_on(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setPayloadOn(value);
}
void channelio_set_payload_off(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setPayloadOff(value);
}
void channelio_set_interval(unsigned char number, int interval) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setIntervalSec(interval);
}

void channelio_set_payload_value(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setPayloadValue(value);
}

void channelio_set_filename(unsigned char number, const char *value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setFileName(value);
}

void channelio_set_execute_on(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setExecuteOn(value);
}

void channelio_set_execute_off(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setExecuteOff(value);
}

void channelio_set_execute(unsigned char number, const char *value) {
  if (channels == NULL) return;
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->setExecute(value);
}

void channelio_set_function(unsigned char number, int function) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setFunction(function);
}

void channelio_set_type(unsigned char number, int type) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setType(type);
}
int channelio_get_type(unsigned char number) {
  if (channels == NULL) return 0;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) return channel->getType();

  return 0;
}
int channelio_get_function(unsigned char number) {
  if (channels == NULL) return 0;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) return channel->getFunction();

  return 0;
}
void channelio_set_toggle(unsigned char number, int toggle) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setToggleSec(toggle);
}

void channelio_set_mqtt_topic_in(unsigned char number, const char *value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setStateTopic(value);
}
void channelio_set_mqtt_topic_out(unsigned char number, const char *value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setCommandTopic(value);
}
void channelio_set_mqtt_retain(unsigned char number, unsigned char value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setRetain(value);
}
void channelio_set_mqtt_template_in(unsigned char number, const char *value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setStateTemplate(value);
}
void channelio_set_mqtt_template_out(unsigned char number, const char *value) {
  if (channels == NULL) return;

  client_device_channel *channel = channels->find_channel(number);

  if (channel) channel->setCommandTemplate(value);
}

void channelio_w1_iterate(void) {
  int a;
  if (!channels->getInitialized()) return;

  client_device_channel *channel;
  for (a = 0; a < channels->getChannelCount(); a++) {
    channel = channels->getChannel(a);

    if (channel->getFileName().length() > 0) {
      if (channelio_read_from_file(channel, 1)) {
        channelio_raise_valuechanged(channel);
      };
    };

    if (channel->getToggleSec() > 0 && !channel->getToggled()) {
      struct timeval now;
      gettimeofday(&now, NULL);

      if (now.tv_sec - channel->getLastSeconds() >= channel->getToggleSec()) {
        supla_log(LOG_DEBUG, "toggling value for channel: %s",
                  channel->getNumber());
        channel->toggleValue();
        channelio_raise_valuechanged(channel);
      };
    };
  }
}

#ifdef __SINGLE_THREAD
void channelio_iterate(void) {
  if (!channels->getInitialized()) return;
  channelio_w1_iterate();
}
#else
void channelio_w1_execute(void *user_data, void *sthread) {
  while (!sthread_isterminated(sthread)) {
    channelio_w1_iterate();
    usleep(W1_TEMP_MINDELAY_USEC);
  }
}
#endif

void channelio_raise_execute_command(client_device_channel *channel) {
  std::string command = channel->getExecute();
  std::string commandOn = channel->getExecuteOn();
  std::string commandOff = channel->getExecuteOff();

  if (command.length() == 0 && commandOn.length() == 0 &&
      commandOff.length() == 0)
    return;

  if (command.length() > 0) {
    int commandResult = system(command.c_str());
    if (commandResult != 0) {
      supla_log(LOG_WARNING, "%s", command.c_str());
      supla_log(LOG_WARNING, "The command above failed with exist status %d",
                commandResult);
    }
  }

  char value[SUPLA_CHANNELVALUE_SIZE];
  channel->getValue(value);

  if (commandOn.length() > 0 && value[0] == 1) {
    int commandResult = system(commandOn.c_str());
    if (commandResult != 0) {
      supla_log(LOG_WARNING, "%s", commandOn.c_str());
      supla_log(LOG_WARNING, "The command above failed with exist status %d",
                commandResult);
    }
  }

  if (commandOn.length() > 0 && value[0] == 0) {
    int commandResult = system(commandOff.c_str());
    if (commandResult != 0) {
      supla_log(LOG_WARNING, "%s", commandOff.c_str());
      supla_log(LOG_WARNING, "The command above failed with exist status %d",
                commandResult);
    }
  }
}

void channelio_get_value(unsigned char number,
                         char value[SUPLA_CHANNELVALUE_SIZE]) {
  client_device_channel *channel = channels->find_channel(number);
  if (channel) channel->getValue(value);
}

void channelio_raise_mqtt_valuechannged(client_device_channel *channel) {
  if (channel->getCommandTopic().length() == 0) return;
  publish_mqtt_message_for_channel(channel);
}

char channelio_set_value(unsigned char number,
                         char value[SUPLA_CHANNELVALUE_SIZE],
                         unsigned int time_ms) {
  client_device_channel *channel = channels->find_channel(number);

  if (channel) {
    channel->setValue(value);

    /* execute command if specified */
    channelio_raise_execute_command(channel);

    /* send value to MQTT if specified */
    channelio_raise_mqtt_valuechannged(channel);

    /* report value back to SUPLA server */
    channelio_raise_valuechanged(channel);

    return true;
  };

  return false;
}

void channelio_channels_to_srd(unsigned char *channel_count,
                               TDS_SuplaDeviceChannel_B *chnl) {
  int a;

  *channel_count = channels->getChannelCount();

  for (a = 0; a < *channel_count; a++) {
    client_device_channel *channel = channels->getChannel(a);
    if (channel) {
      chnl[a].Number = channel->getNumber();
      chnl[a].Type = channel->getType();
      chnl[a].Default = channel->getFunction();
      channel->getValue(chnl[a].value);
    }
  }
}

void channelio_setcalback_on_channel_value_changed(
    _func_channelio_valuechanged on_valuechanged, void *user_data) {
  channels->on_valuechanged = on_valuechanged;
  channels->on_valuechanged_user_data = user_data;

  // MQTT SETUP

  std::string mqtt_host(scfg_string(CFG_MQTT_SERVER));
  if (mqtt_host.length() == 0) return;

  vector<std::string> topics;
  channels->getMqttSubscriptionTopics(&topics);

  supla_log(LOG_DEBUG, "initializing MQTT broker connection...");

  mqtt_client_free();

  mqtt_client_init(std::string(scfg_string(CFG_MQTT_SERVER)),
                   scfg_int(CFG_MQTT_PORT),
                   std::string(scfg_string(CFG_MQTT_USERNAME)),
                   std::string(scfg_string(CFG_MQTT_PASSWORD)),
                   std::string(scfg_string(CFG_MQTT_CLIENT_NAME)), 3, topics,
                   mqtt_subscribe_callback);

  supla_log(LOG_DEBUG, "initialization completed");
}

void tmp_channelio_raise_valuechanged(unsigned char number) {
  client_device_channel *channel = channels->find_channel(number);

  if (channel) {
    channelio_raise_valuechanged(channel);
  }
}
