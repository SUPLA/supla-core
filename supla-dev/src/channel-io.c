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

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "channel-io.h"
#include "eh.h"
#include "gpio.h"
#include "lck.h"
#include "log.h"
#include "mcp23008.h"
#include "safearray.h"
#include "sthread.h"
#include "w1.h"

// TODO(pzygmunt) whole remodel

#define GPIO_MINDELAY_USEC 200000
#ifdef __SINGLE_THREAD
#define W1_TEMP_MINDELAY_SEC 120
#else
#define W1_TEMP_MINDELAY_SEC 30
#endif
#define MCP23008_MINDELAY_SEC 1

typedef struct {
#ifdef __SINGLE_THREAD
  struct timeval close_time;
#endif

  struct timeval last_tv;
  char value;
  void *lck;
} TChannelGpioPortValue;

typedef struct {
  struct timeval last_tv;
  double temp;
  double humidity;
  void *lck;
} TChannelW1TempValue;

typedef struct {
  int color;
  char color_brightness;
  char brightness;
  void *lck;
} TChannelRGBWValue;

typedef struct {
#ifdef __SINGLE_THREAD
  struct timeval close_time;
#endif
  struct timeval last_tv;
  char addr;
  char reset;
  char dir;
  char port;
  char active;
  void *lck;
  TChannelGpioPortValue gpio;
} TChannelMCP23008;

typedef struct {
  unsigned char number;
  int type;
  int driver;
  unsigned char gpio1;
  unsigned char gpio2;
  unsigned char bistable;
  char *w1;

  TChannelGpioPortValue gpio1_value;
  TChannelGpioPortValue gpio2_value;
  TChannelW1TempValue w1_value;
  TChannelRGBWValue rgbw_value;
  TChannelMCP23008 mcp23008;
} TDeviceChannel;

typedef struct {
  TDeviceChannel *channel;
  void *sthread;
  unsigned char port1;
  unsigned char port2;
  unsigned char bistable;
  char hi;
  int time_ms;
  char raise;
} TGpioThreadItem;

typedef struct {
  char initialized;

  TDeviceChannel *channels;
  int channel_count;

  void *cb_lck;
  _func_channelio_valuechanged on_valuechanged;
  void *on_valuechanged_user_data;

  void *wl_lck;
  TGpioPort *watch_list;
  int watch_list_count;

#ifndef __SINGLE_THREAD
  void *gpio_thread_arr;
  void *w1_sthread;
  void *mcp_sthread;
#endif
} TChannelIo;

TChannelIo *cio = NULL;

#ifndef __SINGLE_THREAD
void channelio_w1_execute(void *user_data, void *sthread);
void channelio_gpio_monitor_execute(void *user_data, void *sthread);
void channelio_mcp23008_execute(void *user_data, void *sthread);
#endif

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

char channelio_init(void) {
  cio = malloc(sizeof(TChannelIo));
  if (cio == NULL) return 0;

  memset(cio, 0, sizeof(TChannelIo));
  cio->cb_lck = lck_init();
  cio->wl_lck = lck_init();

#ifndef __SINGLE_THREAD
  cio->gpio_thread_arr = safe_array_init();
#endif

  return 1;
}

void channelio_free(void) {
  int a;
  if (cio) {
#ifndef __SINGLE_THREAD
    // 1st stop threads!!!

    if (cio->w1_sthread) sthread_twf(cio->w1_sthread);

    if (cio->mcp_sthread) sthread_twf(cio->mcp_sthread);

    while (safe_array_count(cio->gpio_thread_arr) > 0) {
      safe_array_lock(cio->gpio_thread_arr);
      TGpioThreadItem *gti = safe_array_get(cio->gpio_thread_arr, 0);

      if (gti && gti->sthread) sthread_terminate(gti->sthread);

      safe_array_unlock(cio->gpio_thread_arr);

      usleep(100);
    }

    safe_array_free(cio->gpio_thread_arr);

#endif

    for (a = 0; a < cio->channel_count; a++) {
      if (cio->channels[a].w1 != NULL) free(cio->channels[a].w1);

      if (channelio_gpio_in(&cio->channels[a], 1) == 0) {
        if (cio->channels[a].gpio1 > 0)
          gpio_set_value(cio->channels[a].gpio1, 0);

        if (cio->channels[a].gpio2 > 0)
          gpio_set_value(cio->channels[a].gpio2, 0);
      }

      lck_free(cio->channels[a].gpio1_value.lck);
      lck_free(cio->channels[a].gpio2_value.lck);
    }

    if (cio->channels) free(cio->channels);

    if (cio->watch_list) free(cio->watch_list);

    lck_free(cio->wl_lck);
    lck_free(cio->cb_lck);

    free(cio);
    cio = NULL;
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
      return 1;
  }

  return 0;
}

void channelio_gpio_port_init(TDeviceChannel *channel,
                              TChannelGpioPortValue *gpio_value,
                              unsigned char number, char in) {
  if (gpio_port_init(number, in, 0) == 0) return;

  if (in == 1) {
    if (gpio_value->lck == NULL) gpio_value->lck = lck_init();

    lck_lock(gpio_value->lck);
    gpio_get_value(number, &gpio_value->value);

#ifdef __SINGLE_THREAD
    gpio_value->close_time.tv_sec = 0;
    gpio_value->close_time.tv_usec = 0;
#endif

    gettimeofday(&gpio_value->last_tv, NULL);

    lck_unlock(gpio_value->lck);

    lck_lock(cio->wl_lck);

    cio->watch_list = realloc(cio->watch_list,
                              sizeof(TGpioPort) * (cio->watch_list_count + 1));
    TGpioPort *port = &cio->watch_list[cio->watch_list_count];
    memset(port, 0, sizeof(TGpioPort));

    port->number = number;
    port->user_data1 = gpio_value;
    port->user_data2 = channel;
    cio->watch_list_count++;

    lck_unlock(cio->wl_lck);
  }
}

char channelio_read_temp_and_humidity(int type, char *w1,
                                      TChannelW1TempValue *w1_value,
                                      char log_err) {
  double temp, humidity;
  struct timeval now;
  char result = 0;
  char read_result = 0;

  if (type != SUPLA_CHANNELTYPE_THERMOMETERDS18B20 &&
      type != SUPLA_CHANNELTYPE_DHT11 && type != SUPLA_CHANNELTYPE_DHT22 &&
      type != SUPLA_CHANNELTYPE_AM2302)
    return 0;

  if (w1 != NULL || type != SUPLA_CHANNELTYPE_THERMOMETERDS18B20) {
    gettimeofday(&now, NULL);

    lck_lock(w1_value->lck);

    if (now.tv_sec - w1_value->last_tv.tv_sec >= W1_TEMP_MINDELAY_SEC) {
      w1_value->last_tv = now;

      switch (type) {
        case SUPLA_CHANNELTYPE_THERMOMETERDS18B20:
          read_result = w1_ds18b20_get_temp(w1, &temp);
          break;
        case SUPLA_CHANNELTYPE_DHT11:
          read_result = w1_dht_read(w1, &temp, &humidity, 11);
          break;
        case SUPLA_CHANNELTYPE_DHT22:
        case SUPLA_CHANNELTYPE_AM2302:
          read_result = w1_dht_read(w1, &temp, &humidity, 22);
          break;
      }

      if (read_result == 1) {
        if (w1_value->temp != temp) {
          w1_value->temp = temp;
          // supla_log(LOG_DEBUG, "Temp: %f", temp);
          result = 1;
        }

        if (w1_value->humidity != humidity) {
          w1_value->humidity = humidity;
          result = 1;
        }

      } else {
        w1_value->temp = -275;
        w1_value->humidity = -1;

        if (log_err == 1)
          supla_log(LOG_ERR, "Can't read 1-wire device %s", w1 ? w1 : "");
      }
    }

    lck_unlock(w1_value->lck);
  }

  return result;
}

char channelio_read_rgbw_values(int type, TChannelRGBWValue *rgbw_value) {
  // TEMPORATY IMPLEMENTATION

  char result = 0;

  if (type != SUPLA_CHANNELTYPE_DIMMER &&
      type != SUPLA_CHANNELTYPE_RGBLEDCONTROLLER &&
      type != SUPLA_CHANNELTYPE_DIMMERANDRGBLED)
    return 0;

  lck_lock(rgbw_value->lck);

  rgbw_value->color = 0x00FF00;
  rgbw_value->brightness = 0;
  rgbw_value->color_brightness = 10;

  result = 1;

  lck_unlock(rgbw_value->lck);

  return result;
}

#ifndef __SINGLE_THREAD

void channelio_gpio_thread_finish(void *user_data, void *sthread) {
  int a;
  TGpioThreadItem *gti;

  safe_array_lock(cio->gpio_thread_arr);
  for (a = 0; a < safe_array_count(cio->gpio_thread_arr); a++) {
    gti = safe_array_get(cio->gpio_thread_arr, a);
    if (gti && gti->sthread == sthread) {
      free(gti);
      safe_array_delete(cio->gpio_thread_arr, a);
      break;
    }
  }

  safe_array_unlock(cio->gpio_thread_arr);
}

char channelio_gpio_thread_exists(unsigned char port) {
  int a;
  char result = 0;

  if (port == 0) return 0;

  TGpioThreadItem *gti;

  safe_array_lock(cio->gpio_thread_arr);
  for (a = 0; a < safe_array_count(cio->gpio_thread_arr); a++) {
    gti = safe_array_get(cio->gpio_thread_arr, a);
    if (gti && gti->port1 == port) {
      result = 1;
      break;
    }
  }

  safe_array_unlock(cio->gpio_thread_arr);

  return result;
}

char channelio_start_gpio_thread(TDeviceChannel *channel,
                                 _func_sthread_execute execute, char hi,
                                 unsigned char port1, unsigned char port2,
                                 unsigned char bistable, int time_ms,
                                 char raise) {
  TGpioThreadItem *gti = NULL;

  if (channelio_gpio_thread_exists(port1)) return 0;

  gti = malloc(sizeof(TGpioThreadItem));
  memset(gti, 0, sizeof(TGpioThreadItem));
  gti->channel = channel;
  gti->hi = hi;
  gti->port1 = port1;
  gti->port2 = port2;
  gti->bistable = bistable;
  gti->time_ms = time_ms;
  gti->raise = raise;

  Tsthread_params p;
  p.user_data = gti;
  p.free_on_finish = 1;
  p.execute = execute;
  p.finish = channelio_gpio_thread_finish;
  p.initialize = NULL;

  safe_array_lock(cio->gpio_thread_arr);
  gti->sthread = sthread_run(&p);
  safe_array_add(cio->gpio_thread_arr, gti);
  safe_array_unlock(cio->gpio_thread_arr);

  return 1;
}
#endif

void channelio_channel_init(void) {
  int a;
  TDeviceChannel *channel;

  if (cio->initialized == 1) return;

  cio->initialized = 1;

  for (a = 0; a < cio->channel_count; a++) {
    channel = &cio->channels[a];
    if (channel->gpio1 > 0)
      channelio_gpio_port_init(channel, &channel->gpio1_value, channel->gpio1,
                               channelio_gpio_in(channel, 1));

    if (channel->gpio2 > 0)
      channelio_gpio_port_init(channel, &channel->gpio2_value, channel->gpio2,
                               channelio_gpio_in(channel, 2));

    channelio_read_temp_and_humidity(channel->type, channel->w1,
                                     &channel->w1_value, 1);

    channelio_read_rgbw_values(channel->type, &channel->rgbw_value);

    if (channel->driver == SUPLA_CHANNELDRIVER_MCP23008) {
      mcp23008_init(channel->mcp23008.reset, channel->mcp23008.addr);
      if (channel->mcp23008.active) {
        mcp23008_gpio_port_init(channel->mcp23008.port, channel->mcp23008.dir,
                                channel->mcp23008.gpio.value);
      }
    }
  }

#ifndef __SINGLE_THREAD
  cio->w1_sthread = sthread_simple_run(channelio_w1_execute, NULL, 0);
  lck_lock(cio->wl_lck);

  if (cio->watch_list_count > 0) {
    channelio_start_gpio_thread(NULL, channelio_gpio_monitor_execute, 1, 0, 0,
                                0, 0, 0);
  }

  lck_unlock(cio->wl_lck);

  cio->mcp_sthread = sthread_simple_run(channelio_mcp23008_execute, NULL, 0);
#endif
}

int channelio_channel_count(void) { return cio->channel_count; }

TDeviceChannel *channelio_find(unsigned char number, char create) {
  int a;
  TDeviceChannel *result = NULL;

  for (a = 0; a < cio->channel_count; a++)
    if (cio->channels[a].number == number) return &cio->channels[a];

  if (cio->initialized == 0 && create == 1 &&
      cio->channel_count < SUPLA_CHANNELMAXCOUNT) {
    cio->channels = realloc(cio->channels,
                            sizeof(TDeviceChannel) * (cio->channel_count + 1));
    result = &cio->channels[cio->channel_count];
    cio->channel_count++;

    memset(result, 0, sizeof(TDeviceChannel));
    result->number = number;
  }

  return result;
}

void channelio_set_type(unsigned char number, int type) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 1) return;

  if (channelio_allowed_type(type) != 1) {
    supla_log(LOG_WARNING, "Channel %i is the wrong type %i", number, type);
    return;
  }

  channel = channelio_find(number, 1);

  if (channel != NULL) channel->type = type;
}

int channelio_get_type(unsigned char number) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 0) return 0;

  channel = channelio_find(number, 0);

  if (channel != NULL) return channel->type;

  return 0;
}

void channelio_set_gpio1(unsigned char number, unsigned char gpio1) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 1) return;

  channel = channelio_find(number, 1);

  if (channel != NULL) channel->gpio1 = gpio1;
}

void channelio_set_gpio2(unsigned char number, unsigned char gpio2) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 1) return;

  channel = channelio_find(number, 1);

  if (channel != NULL) channel->gpio2 = gpio2;
}

void channelio_set_bistable_flag(unsigned char number, unsigned char bistable) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 1) return;

  channel = channelio_find(number, 1);

  if (channel != NULL) channel->bistable = bistable;
}

void channelio_set_w1(unsigned char number, const char *w1) {
  TDeviceChannel *channel;

  if (cio == NULL || cio->initialized == 1 || w1 == NULL || strlen(w1) == 0)
    return;

  channel = channelio_find(number, 1);

  if (channel != NULL) channel->w1 = strdup(w1);
}

#define HELPER_CHANNEL_CHECK_VALUE_SET(PARAM, NUMBER, VALUE) \
  do {                                                       \
    TDeviceChannel *channel;                                 \
    if (cio == NULL || cio->initialized == 1) return;        \
    channel = channelio_find(NUMBER, 1);                     \
    if (channel != NULL) channel->PARAM = VALUE;             \
  } while (0)

void channelio_set_mcp23008_driver(unsigned char number, int driver) {
  HELPER_CHANNEL_CHECK_VALUE_SET(driver, number, driver);
}

void channelio_set_mcp23008_addr(unsigned char number, unsigned char addr) {
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.addr, number, addr);
}

void channelio_set_mcp23008_gpio_dir(unsigned char number,
                                     unsigned char value) {
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.dir, number, value);
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.active, number, 1);
}

void channelio_set_mcp23008_gpio_val(unsigned char number,
                                     unsigned char value) {
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.gpio.value, number, value);
}

void channelio_set_mcp23008_gpio_port(unsigned char number,
                                      unsigned char port) {
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.port, number, port);
}

void channelio_set_mcp23008_reset(unsigned char number, unsigned char reset) {
  HELPER_CHANNEL_CHECK_VALUE_SET(mcp23008.reset, number, reset);
}

char channelio_get_cvalue(TDeviceChannel *channel,
                          char value[SUPLA_CHANNELVALUE_SIZE]) {
  memset(value, 0, SUPLA_CHANNELVALUE_SIZE);

  if (channel) {
    if (channel->type == SUPLA_CHANNELTYPE_SENSORNO ||
        channel->type == SUPLA_CHANNELTYPE_SENSORNC ||
        channel->type == SUPLA_CHANNELTYPE_RELAYHFD4 ||
        channel->type == SUPLA_CHANNELTYPE_RELAYG5LA1A) {
      if (channel->bistable == 1) {
        lck_lock(channel->gpio2_value.lck);
        value[0] = channel->gpio2_value.value;
        lck_unlock(channel->gpio2_value.lck);

      } else {
        lck_lock(channel->gpio1_value.lck);
        value[0] = channel->gpio1_value.value;
        if (channel->driver == SUPLA_CHANNELDRIVER_MCP23008 &&
            (channel->type == SUPLA_CHANNELTYPE_SENSORNO ||
             channel->type == SUPLA_CHANNELTYPE_SENSORNC)) {
          value[0] = channel->mcp23008.gpio.value;
        }
        lck_unlock(channel->gpio1_value.lck);
      }

      return 1;
    }

    if (channel->type == SUPLA_CHANNELTYPE_2XRELAYG5LA1A) {
      lck_lock(channel->gpio1_value.lck);
      value[0] = channel->gpio1_value.value;
      lck_unlock(channel->gpio1_value.lck);

      if (value[0] == 0) {
        lck_lock(channel->gpio2_value.lck);
        value[0] = channel->gpio2_value.value;
        lck_unlock(channel->gpio2_value.lck);

        if (value[0] != 0) {
          value[0] = 2;
        }
      } else {
        value[0] = 1;
      }

      return 1;
    }

    if (channel->type == SUPLA_CHANNELTYPE_THERMOMETERDS18B20) {
      assert(sizeof(double) <= SUPLA_CHANNELVALUE_SIZE);

      lck_lock(channel->w1_value.lck);
      memcpy(value, &channel->w1_value.temp, sizeof(double));
      lck_unlock(channel->w1_value.lck);

      return 1;
    }

    if (channel->type == SUPLA_CHANNELTYPE_DHT11 ||
        channel->type == SUPLA_CHANNELTYPE_DHT22 ||
        channel->type == SUPLA_CHANNELTYPE_AM2302) {
      assert(sizeof(double) <= SUPLA_CHANNELVALUE_SIZE);

      lck_lock(channel->w1_value.lck);

      int t = channel->w1_value.temp * 1000.00;
      int h = channel->w1_value.humidity * 1000.00;

      memcpy(value, &t, 4);
      memcpy(&value[4], &h, 4);

      lck_unlock(channel->w1_value.lck);

      return 1;
    }

    if (channel->type == SUPLA_CHANNELTYPE_DIMMER ||
        channel->type == SUPLA_CHANNELTYPE_RGBLEDCONTROLLER ||
        channel->type == SUPLA_CHANNELTYPE_DIMMERANDRGBLED) {
      lck_lock(channel->rgbw_value.lck);

      value[0] = channel->rgbw_value.brightness;

      value[1] = channel->rgbw_value.color_brightness;
      value[2] = (char)((channel->rgbw_value.color & 0x000000FF));  // BLUE
      value[3] =
          (char)((channel->rgbw_value.color & 0x0000FF00) >> 8);  // GREEN
      value[4] = (char)((channel->rgbw_value.color & 0x00FF0000) >> 16);  // RED

      lck_unlock(channel->rgbw_value.lck);

      return 1;
    }
  }

  return 0;
}

char channelio_get_value(unsigned char number,
                         char value[SUPLA_CHANNELVALUE_SIZE]) {
  return channelio_get_cvalue(channelio_find(number, 0), value);
}

void channelio_raise_valuechanged(TDeviceChannel *channel) {
  char value[SUPLA_CHANNELVALUE_SIZE];

  if (channelio_get_cvalue(channel, value)) {
    lck_lock(cio->cb_lck);
    if (cio->on_valuechanged)
      cio->on_valuechanged(channel->number, value,
                           cio->on_valuechanged_user_data);

    lck_unlock(cio->cb_lck);
  }
}

void channelio_gpio_on_portvalue(TGpioPort *port) {
  TChannelGpioPortValue *gpio_value;
  struct timeval now;
  char _raise = 0;

  gpio_value = (TChannelGpioPortValue *)port->user_data1;

  lck_lock(gpio_value->lck);

  if (gpio_value->value != port->value) {
    gettimeofday(&now, NULL);

    if (gpio_value->last_tv.tv_sec > now.tv_sec  // the date has been changed
        ||
        (long int)(now.tv_sec - gpio_value->last_tv.tv_sec) >
            (long int)(GPIO_MINDELAY_USEC / 1000000) ||
        ((long int)(now.tv_sec - gpio_value->last_tv.tv_sec) ==
             (long int)(GPIO_MINDELAY_USEC / 1000000) &&
         (long int)(now.tv_usec - gpio_value->last_tv.tv_usec) >=
             (long int)(GPIO_MINDELAY_USEC % 1000000))) {
      gpio_value->value = port->value;
      gpio_value->last_tv = now;
      _raise = 1;
    }
  }

  lck_unlock(gpio_value->lck);

  if (_raise == 1)
    channelio_raise_valuechanged((TDeviceChannel *)port->user_data2);
}
#ifdef __SINGLE_THREAD
char channelio__gpio_set_timed_lo_value(TDeviceChannel *channel,
                                        unsigned char port_number,
                                        TChannelGpioPortValue *gpio_value,
                                        struct timeval *now, char raise) {
  char result = 0;

  if (port_number > 0) {
    lck_lock(gpio_value->lck);

    if (gpio_value->close_time.tv_sec > 0 ||
        gpio_value->close_time.tv_usec > 0) {
      result = 1;

      if (timercmp(now, &gpio_value->close_time, >=)) {
        gpio_value->close_time.tv_sec = 0;
        gpio_value->close_time.tv_usec = 0;
        gpio_value->value = 0;

        lck_unlock(gpio_value->lck);

        // supla_log(LOG_DEBUG, "Port %i is set to LO", port_number);
        gpio_set_value(port_number, 0);
        if (raise) channelio_raise_valuechanged(channel);

        return result;
      }
    }

    lck_unlock(gpio_value->lck);
  }

  return result;
}

char channelio_gpio_set_timed_lo_value(void) {
  int a;
  char result;
  struct timeval now;

  gettimeofday(&now, NULL);
  result = 0;

  for (a = 0; a < cio->channel_count; a++) {
    if (1 == channelio__gpio_set_timed_lo_value(
                 &cio->channels[a], cio->channels[a].gpio1,
                 &cio->channels[a].gpio1_value, &now, 1)) {
      result = 1;
    }

    if (1 == channelio__gpio_set_timed_lo_value(
                 &cio->channels[a], cio->channels[a].gpio2,
                 &cio->channels[a].gpio2_value, &now, result == 0 ? 1 : 0)) {
      result = 1;
    }
  }

  return result;
}
#endif
void channelio_gpio_iterate(void) {
  int a;
  TChannelGpioPortValue *gpio_value;

  lck_lock(cio->wl_lck);

  if (cio->watch_list_count > 0) {
    for (a = 0; a < cio->watch_list_count; a++) {
      gpio_value = (TChannelGpioPortValue *)cio->watch_list->user_data1;
      lck_lock(gpio_value->lck);
      cio->watch_list->value = gpio_value->value;
      lck_unlock(gpio_value->lck);
    }

#ifdef __SINGLE_THREAD
    gpio_wait(cio->watch_list, cio->watch_list_count, 100,
              channelio_gpio_on_portvalue);
    channelio_gpio_set_timed_lo_value();
#else

    gpio_wait(cio->watch_list, cio->watch_list_count, 5000000,
              channelio_gpio_on_portvalue);
#endif
  }

  lck_unlock(cio->wl_lck);
}

void channelio_w1_iterate(void) {
  int a;

  for (a = 0; a < cio->channel_count; a++) {
    if (channelio_read_temp_and_humidity(cio->channels[a].type,
                                         cio->channels[a].w1,
                                         &cio->channels[a].w1_value, 0) == 1)
      channelio_raise_valuechanged(&cio->channels[a]);
  }
}

void channelio_mcp23008_iterate(void) {
  int a;
  for (a = 0; a < cio->channel_count; a++) {
    TDeviceChannel *ch = &cio->channels[a];
    if (ch->type == SUPLA_CHANNELTYPE_SENSORNO ||
        ch->type == SUPLA_CHANNELTYPE_SENSORNC) {
      unsigned char val = mcp23008_gpio_get_value(ch->mcp23008.port);

      if (ch->mcp23008.gpio.value != val) {
        ch->mcp23008.gpio.value = val;
        channelio_raise_valuechanged(ch);
      }
    }
  }
}

#ifdef __SINGLE_THREAD
void channelio_iterate(void) {
  if (cio->initialized == 0) return;

  channelio_gpio_iterate();
  channelio_w1_iterate();
  channelio_mcp23008_iterate();
}
#else

void channelio_w1_execute(void *user_data, void *sthread) {
  int wait = (W1_TEMP_MINDELAY_SEC * 1000000) - 2000000;

  if (wait <= 0) wait = 1000000;

  while (!sthread_isterminated(sthread)) {
    channelio_w1_iterate();
    usleep(wait);
  }
}

void channelio_mcp23008_execute(void *user_data, void *sthread) {
  int wait = (MCP23008_MINDELAY_SEC * 1000000) - 2000000;

  if (wait <= 0) wait = 1000000;

  while (!sthread_isterminated(sthread)) {
    channelio_mcp23008_iterate();
    usleep(wait);
  }
}

void channelio_gpio_monitor_execute(void *user_data, void *sthread) {
  while (!sthread_isterminated(sthread)) {
    channelio_gpio_iterate();
  }
}

void channelio_gpio_set_hi_execute(void *user_data, void *sthread) {
  TGpioThreadItem *gti = (TGpioThreadItem *)user_data;

  TChannelGpioPortValue *gpio1_value = gti->port1 == gti->channel->gpio1
                                           ? &gti->channel->gpio1_value
                                           : &gti->channel->gpio2_value;

  char bistable = gti->port2 != 0 && gti->bistable == 1;
  char gpio2_hi = 0;

  if (bistable == 1) {
    lck_lock(gti->channel->gpio2_value.lck);
    gpio2_hi = gti->channel->gpio2_value.value;
    lck_unlock(gti->channel->gpio2_value.lck);
  }

  if (bistable == 0 || gti->hi != gpio2_hi) {
    lck_lock(gpio1_value->lck);
    gpio1_value->value = 1;
    gpio_set_value(gti->port1, 1);
    lck_unlock(gpio1_value->lck);

    if (bistable == 1) {
      usleep(500000);

      lck_lock(gpio1_value->lck);
      gpio1_value->value = 0;
      gpio_set_value(gti->port1, 0);
      lck_unlock(gpio1_value->lck);
    }
  }

  if (gti->raise) channelio_raise_valuechanged(gti->channel);

  if (gti->time_ms > 0) {
    usleep(gti->time_ms * 1000);

    char v;

    if (bistable == 1) {
      lck_lock(gti->channel->gpio2_value.lck);
      gpio2_hi = gti->channel->gpio2_value.value;
      lck_unlock(gti->channel->gpio2_value.lck);

      v = 1;

    } else {
      v = 0;
    }

    if (bistable == 0 || gti->hi == gpio2_hi) {
      lck_lock(gpio1_value->lck);
      gpio1_value->value = v;
      gpio_set_value(gti->port1, v);
      lck_unlock(gpio1_value->lck);

      if (bistable == 1) {
        usleep(500000);

        lck_lock(gpio1_value->lck);
        gpio1_value->value = 0;
        gpio_set_value(gti->port1, 0);
        lck_unlock(gpio1_value->lck);
      }

      if (gti->raise) channelio_raise_valuechanged(gti->channel);
    }
  }
}

#endif

char channelio__set_hi_value(TDeviceChannel *channel, char hi,
                             unsigned int time_ms, char second_gpio,
                             char raise) {
  char result = 0;
  unsigned char port1;
  unsigned char port2 = 0;
  TChannelGpioPortValue *gpio_value;

  if (second_gpio == 0) {
    gpio_value = &channel->gpio1_value;
    port1 = channel->gpio1;
    port2 = channel->gpio2;
  } else {
    gpio_value = &channel->gpio2_value;
    port1 = channel->gpio2;
  }

  if (channel->type == SUPLA_CHANNELTYPE_RELAYHFD4 ||
      channel->type == SUPLA_CHANNELTYPE_RELAYG5LA1A ||
      channel->type == SUPLA_CHANNELTYPE_2XRELAYG5LA1A) {
    if (hi == 0) time_ms = 0;

    if (hi > 0 && channel->type == SUPLA_CHANNELTYPE_RELAYHFD4) {
      hi = 1;
    }

#ifdef __SINGLE_THREAD

    // BISTABLE RELAY UNSUPPORTED

    lck_lock(gpio_value->lck);

    if (time_ms == 0 || hi == 0) {
      gpio_value->close_time.tv_sec = 0;
      gpio_value->close_time.tv_usec = 0;
    } else if (hi == 1) {
      gettimeofday(&gpio_value->close_time, NULL);
      gpio_value->close_time.tv_sec += (int)(time_ms / 1000);
      gpio_value->close_time.tv_usec += (int)((time_ms % 1000) * 1000);
    }

    gpio_value->value = hi == 0 ? 0 : 1;
    gpio_set_value(port, gpio_value->value);
    lck_unlock(gpio_value->lck);

    if (raise) channelio_raise_valuechanged(channel);

    result = 1;

#else

    if (time_ms > 0 || channel->bistable) {
      result = channelio_start_gpio_thread(
          channel, channelio_gpio_set_hi_execute, hi, port1, port2,
          channel->bistable, time_ms, raise);
    } else {
      if (channel->driver == SUPLA_CHANNELDRIVER_MCP23008) {
        lck_lock(gpio_value->lck);
        mcp23008_gpio_set_value(channel->mcp23008.port, !hi);
        gpio_value->value = hi == 0 ? 0 : 1;
        lck_unlock(gpio_value->lck);
      } else {
        lck_lock(gpio_value->lck);
        gpio_value->value = hi == 0 ? 0 : 1;
        gpio_set_value(port1, gpio_value->value);
        lck_unlock(gpio_value->lck);
      }

      if (raise) channelio_raise_valuechanged(channel);

      result = 1;
    }

#endif

    // supla_log(LOG_DEBUG, "Port %i is set to HI", channel->gpio1);
  }

  return result;
}

char channelio_set_hi_value(unsigned char number, char hi,
                            unsigned int time_ms) {
  char result = 0;

  TDeviceChannel *channel = channelio_find(number, 0);

  if (channel) {
    if (channel->type == SUPLA_CHANNELTYPE_2XRELAYG5LA1A) {
      if (hi == 0) {
        if (channelio__set_hi_value(channel, 0, time_ms, 0, 0)) result = 1;

        if (channelio__set_hi_value(channel, 0, time_ms, 1, 1)) result = 1;

      } else if (hi == 1) {
        if (channelio__set_hi_value(channel, 0, time_ms, 1, 0)) result = 1;

        usleep(100000);  // Wait for relay

        if (channelio__set_hi_value(channel, 1, time_ms, 0, 1)) result = 1;

      } else if (hi == 2) {
        if (channelio__set_hi_value(channel, 0, time_ms, 0, 0)) result = 1;

        usleep(100000);  // Wait for relay

        if (channelio__set_hi_value(channel, 1, time_ms, 1, 1)) result = 1;
      }

    } else {
      return channelio__set_hi_value(channel, hi == 0 ? 0 : 1, time_ms, 0, 1);
    }
  }

  return result;
}

char channelio_get_hi_value(unsigned char number, char *hi) {
  char value[SUPLA_CHANNELVALUE_SIZE];

  TDeviceChannel *channel = channelio_find(number, 0);

  if (channel && (channel->type == SUPLA_CHANNELTYPE_RELAYHFD4 ||
                  channel->type == SUPLA_CHANNELTYPE_RELAYG5LA1A ||
                  channel->type == SUPLA_CHANNELTYPE_2XRELAYG5LA1A)) {
    if (channelio_get_value(number, value) == 1) {
      *hi = value[0] == 1 ? 1 : 0;
      return 1;
    }
  }

  return 0;
}

void channelio_channels_to_srd(unsigned char *channel_count,
                               TDS_SuplaDeviceChannel_B *channels) {
  int a;
  *channel_count = cio->channel_count;

  for (a = 0; a < cio->channel_count; a++) {
    channels[a].Number = cio->channels[a].number;
    channels[a].Type = cio->channels[a].type;

    if (channelio_get_cvalue(&cio->channels[a], channels[a].value) == 0) {
      memset(channels[a].value, 0, SUPLA_CHANNELVALUE_SIZE);
    }
  }
}

void channelio_setcalback_on_channel_value_changed(
    _func_channelio_valuechanged on_valuechanged, void *user_data) {
  lck_lock(cio->cb_lck);
  cio->on_valuechanged = on_valuechanged;
  cio->on_valuechanged_user_data = user_data;
  lck_unlock(cio->cb_lck);
}

void tmp_channelio_raise_valuechanged(unsigned char number) {
  TDeviceChannel *channel = channelio_find(number, 0);

  if (channel) {
    channelio_raise_valuechanged(channel);
  }
}
